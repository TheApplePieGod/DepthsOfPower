#include <DepthsOfPower/gui/widget_manager.h>

void widget_manager::AddWidget(const char* name, widget* newWidget)
{
    parentWidgets[name] = newWidget;
}

void widget_manager::RemoveWidget(const char* name, bool deletePointer)
{
    if (deletePointer)
        delete parentWidgets[name];
    parentWidgets[name] = nullptr;
}

void widget_manager::DrawWidget(const char* name)
{
    if (parentWidgets.count(name) != 0)
        parentWidgets[name]->Draw(true);
}

void widget_manager::DrawAllWidgets()
{
    for (const auto &p : parentWidgets)
    {
        parentWidgets[p.first]->Draw(true);
    }
}