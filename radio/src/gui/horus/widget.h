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

#ifndef _WIDGET_H_
#define _WIDGET_H_

#include <inttypes.h>
#include <string.h>

#define MAX_WIDGET_OPTIONS             5

struct Zone
{
  uint16_t x, y, w, h;
};

#define LEN_ZONE_OPTION_STRING         8
union ZoneOptionValue
{
  uint32_t unsignedValue;
  int32_t signedValue;
  bool boolValue;
  char stringValue[LEN_ZONE_OPTION_STRING];
};

#if defined(_MSC_VER)
  #define OPTION_DEFAULT_VALUE_UNSIGNED(x)    uint32_t(x)
  #define OPTION_DEFAULT_VALUE_SIGNED(x)      uint32_t(x)
  #define OPTION_DEFAULT_VALUE_BOOL(x)        uint32_t(x)
  #define OPTION_DEFAULT_VALUE_STRING(...)    *(ZoneOptionValue *)(const char *)__VA_ARGS__
#else
  #define OPTION_DEFAULT_VALUE_UNSIGNED(x)    { .unsignedValue = (x) }
  #define OPTION_DEFAULT_VALUE_SIGNED(x)      { .signedValue = (x) }
  #define OPTION_DEFAULT_VALUE_BOOL(x)        { .boolValue = (x) }
  #define OPTION_DEFAULT_VALUE_STRING(...)    *(ZoneOptionValue *)(const char *)__VA_ARGS__
#endif

struct ZoneOption
{
  enum Type {
    Integer,
    Source,
    Bool,
    String,
    File,
    TextSize,
    Timer,
    Switch,
    Color
  };

  const char * name;
  Type type;
  ZoneOptionValue deflt;
};

class WidgetFactory;
class Widget
{
  public:
    struct PersistentData {
      ZoneOptionValue options[MAX_WIDGET_OPTIONS];
    };

    Widget(const WidgetFactory * factory, const Zone & zone, PersistentData * persistentData):
      factory(factory),
      zone(zone),
      persistentData(persistentData)
    {
    }

    virtual ~Widget()
    {
    }

    virtual void update() const
    {
    }

    inline const WidgetFactory * getFactory() const
    {
        return factory;
    }

    inline const ZoneOption * getOptions() const;

    inline ZoneOptionValue * getOptionValue(unsigned int index) const
    {
      return &persistentData->options[index];
    }

    virtual void refresh() = 0;

    virtual void background()
    {
    }

  protected:
    const WidgetFactory * factory;
    Zone zone;
    PersistentData * persistentData;
};

void registerWidget(const WidgetFactory * factory);

class WidgetFactory
{
  public:
    WidgetFactory(const char * name, const ZoneOption * options=NULL):
      name(name),
      options(options)
    {
      registerWidget(this);
    }

    inline const char * getName() const
    {
        return name;
    }

    inline const ZoneOption * getOptions() const
    {
      return options;
    }

    void initPersistentData(Widget::PersistentData * persistentData) const
    {
      memset(persistentData, 0, sizeof(Widget::PersistentData));
      if (options) {
        int i = 0;
        for (const ZoneOption * option = options; option->name; option++) {
          // TODO compiler bug? The CPU freezes ... persistentData->options[i++] = option->deflt;
          memcpy(&persistentData->options[i++], &option->deflt, sizeof(ZoneOptionValue));
        }
      }
    }

    virtual Widget * create(const Zone & zone, Widget::PersistentData * persistentData, bool init=true) const = 0;

  protected:
    const char * name;
    const ZoneOption * options;
};

template<class T>
class BaseWidgetFactory: public WidgetFactory
{
  public:
    BaseWidgetFactory(const char * name, const ZoneOption * options):
      WidgetFactory(name, options)
    {
    }

    virtual Widget * create(const Zone & zone, Widget::PersistentData * persistentData, bool init=true) const
    {
      if (init) {
        initPersistentData(persistentData);
      }

      return new T(this, zone, persistentData);
    }
};

inline const ZoneOption * Widget::getOptions() const
{
  return getFactory()->getOptions();
}

#define MAX_REGISTERED_WIDGETS 10
extern unsigned int countRegisteredWidgets;
extern const WidgetFactory * registeredWidgets[MAX_REGISTERED_WIDGETS];
Widget * loadWidget(const char * name, const Zone & zone, Widget::PersistentData * persistentData);

#endif // _WIDGET_H_
