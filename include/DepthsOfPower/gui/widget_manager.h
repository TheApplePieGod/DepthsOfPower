#pragma once
#include <map>
#include <string>
#include <DepthsOfPower/gui/widget.h>

// wrapper class to maintain all widgets
class widget_manager
{
public:
    void DrawWidget(const char* name);
    void DrawAllWidgets();
    void AddWidget(const char* name, widget* newWidget);
    void RemoveWidget(const char* name, bool deletePointer);

    template<typename T>
    T* GetWidget(const char* name)
    {
        static_assert(std::is_base_of<widget, T>::value);
        return (T*)parentWidgets[name];
    }

private:
    std::map<std::string, widget*> parentWidgets;

};