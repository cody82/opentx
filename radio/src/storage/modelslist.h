/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _MODELSLIST_H_
#define _MODELSLIST_H_

#include <list>
#include "sdcard.h"

#define MODELCELL_WIDTH                151
#define MODELCELL_HEIGHT               59

class ModelCell
{
  public:
    ModelCell(const char * name):
      buffer(BMP_RGB565, MODELCELL_WIDTH, MODELCELL_HEIGHT)
    {
      buffer.clear(TEXT_BGCOLOR);

      strncpy(this->name, name, sizeof(this->name));

      ModelHeader header;
      const char * error = readModel(name, (uint8_t *)&header, sizeof(header));
      if (error) {
        buffer.drawText(5, 2, "(Invalid Model)", TEXT_COLOR);
        buffer.drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
      }
      else {
        char timer[LEN_TIMER_STRING];
        buffer.drawSizedText(5, 2, header.name, LEN_MODEL_NAME, ZCHAR|TEXT_COLOR);
        getTimerString(timer, 0);
        buffer.drawText(101, 40, timer, TEXT_COLOR);
        for (int i=0; i<4; i++) {
          buffer.drawBitmapPattern(104+i*11, 25, LBM_SCORE0, TITLE_BGCOLOR);
        }
        GET_FILENAME(filename, BITMAPS_PATH, header.bitmap, BITMAPS_EXT);
        const BitmapBuffer * bitmap = BitmapBuffer::load(filename);
        if (bitmap) {
          buffer.drawScaledBitmap(bitmap, 5, 24, 56, 32);
          delete bitmap;
        }
        else {
          buffer.drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
        }
      }
      buffer.drawSolidHorizontalLine(5, 19, 143, LINE_COLOR);
    }

    char name[LEN_MODEL_FILENAME+1];
    BitmapBuffer buffer;
};

class ModelsCategory: public std::list<ModelCell *>
{
  public:
    ModelsCategory(const char * name)
    {
      strncpy(this->name, name, sizeof(this->name));
    }

    ModelCell * addModel(const char * name)
    {
      ModelCell * result = new ModelCell(name);
      push_back(result);
      return result;
    }

    void removeModel(ModelCell * model)
    {
      delete model;
      remove(model);
    }

    void save(FIL * file)
    {
      f_puts("[", file);
      f_puts(name, file);
      f_puts("]", file);
      f_putc('\n', file);
      for (std::list<ModelCell *>::iterator it = begin(); it != end(); ++it) {
        f_puts((*it)->name, file);
        f_putc('\n', file);
      }
    }

    char name[LEN_MODEL_FILENAME+1];
};

class ModelsList
{
  public:
    ModelsList()
    {
    }

    ~ModelsList()
    {
      clear();
    }

    void clear()
    {
      for (std::list<ModelsCategory *>::iterator it = categories.begin(); it != categories.end(); ++it) {
        delete *it;
      }
      categories.clear();
      currentCategory = NULL;
      currentModel = NULL;
    }

    bool load()
    {
      char line[LEN_MODEL_FILENAME+1];
      ModelsCategory * category = NULL;

      clear();

      FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_OPEN_EXISTING | FA_READ);
      if (result != FR_OK) {
        return false;
      }

      while (readNextLine(line, LEN_MODEL_FILENAME)) {
        int len = strlen(line); // TODO could be returned by readNextLine
        if (len > 2 && line[0] == '[' && line[len-1] == ']') {
          line[len-1] = '\0';
          category = new ModelsCategory(&line[1]);
          categories.push_back(category);
        }
        else if (category && len > 0) {
          ModelCell * model = new ModelCell(line);
          category->push_back(model);
          if (!strncmp(line, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME)) {
            currentCategory = category;
            currentModel = model;
          }
        }
      }

      f_close(&file);
      return true;
    }

    void save()
    {
      FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_CREATE_ALWAYS | FA_WRITE);
      if (result != FR_OK) {
        return;
      }

      for (std::list<ModelsCategory *>::iterator it = categories.begin(); it != categories.end(); ++it) {
        (*it)->save(&file);
      }

      f_close(&file);
    }

    bool readNextLine(char * line, int maxlen)
    {
      char c;
      unsigned int read;
      int len=0;
      while (1) {
        FRESULT result = f_read(&file, (uint8_t *)&c, 1, &read);
        if (result != FR_OK || read != 1) {
          line[len] = '\0';
          return false;
        }
        if (c == '\n') {
          if (len > 0) {
            // we skip empty lines
            line[len] = '\0';
            return true;
          }
        }
        else if (c != '\r' && len < maxlen) {
          line[len++] = c;
        }
      }
    }

  ModelsCategory * createCategory()
  {
    ModelsCategory * result = new ModelsCategory("Category");
    categories.push_back(result);
    save();
    return result;
  }

  ModelCell * addModel(ModelsCategory * category, const char * name)
  {
    ModelCell * result = category->addModel(name);
    save();
    return result;
  }

  void removeCategory(ModelsCategory * category)
  {
    delete category;
    categories.remove(category);
  }

  void removeModel(ModelsCategory * category, ModelCell * model)
  {
    category->removeModel(model);
    save();
  }

  std::list<ModelsCategory *> categories;
  ModelsCategory * currentCategory;
  ModelCell * currentModel;

  protected:
    FIL file;
};

#endif // _MODELSLIST_H_
