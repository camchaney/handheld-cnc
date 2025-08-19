#include "menu.h"
#include <Arduino.h>

namespace CompassMenu
{

    MenuItem makeSubmenu(const char *label, Menu *submenu)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Submenu;
        it.data.submenu.ptr = submenu;
        return it;
    }

    MenuItem makeBack(const char *label)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Back;
        return it;
    }

    MenuItem makeAction(const char *label, MenuCallback cb, void *ctx)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Action;
        it.data.action.cb = cb;
        it.data.action.ctx = ctx;
        return it;
    }

    MenuItem makeBool(const char *label, bool *value, bool wrap)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Bool;
        it.data.boolean.value = value;
        it.data.boolean.wrap = wrap;
        return it;
    }

    MenuItem makeInt(const char *label, int *value, int minVal, int maxVal, int step, const char *unitOfMeasure)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Int;
        it.data.integer.value = value;
        it.data.integer.minVal = minVal;
        it.data.integer.maxVal = maxVal;
        it.data.integer.step = step;
        it.data.integer.unitOfMeasure = unitOfMeasure;
        return it;
    }

    MenuItem makeFloat(const char *label, float *value, float minVal, float maxVal, float step, const char *unitOfMeasure)
    {
        MenuItem it;
        it.label = label;
        it.type = MenuItemType::Float;
        it.data.floating.value = value;
        it.data.floating.minVal = minVal;
        it.data.floating.maxVal = maxVal;
        it.data.floating.step = step;
        it.data.floating.unitOfMeasure = unitOfMeasure;
        return it;
    }

    void focusNext(MenuRoot &root)
    {
        if (!root.current || root.current->itemCount == 0) return;
        root.focusIndex = (root.focusIndex + 1) % root.current->itemCount;
        root.changed = true;
    }

    void focusPrev(MenuRoot &root)
    {
        if (!root.current || root.current->itemCount == 0) return;
        root.focusIndex = (root.focusIndex + root.current->itemCount - 1) % root.current->itemCount;
        root.changed = true;
    }

    void back(MenuRoot &root)
    {
        if (root.editState.editing) return;
        if (!root.current || !root.current->parent) return;

        root.current = root.current->parent;
        if (root.current->itemCount)
            root.focusIndex = 0;
        root.changed = true;
    }

    void enter(MenuRoot &root)
    {
        if (!root.current || root.focusIndex >= root.current->itemCount) return;
        MenuItem &it = root.current->items[root.focusIndex];

        switch (it.type)
        {
        case MenuItemType::Submenu:
            if (it.data.submenu.ptr)
            {
                it.data.submenu.ptr->calledFrom = &it;
                root.current = it.data.submenu.ptr;
                root.focusIndex = 0;
            }
            break;

        case MenuItemType::Back:
            back(root);
            break;

        case MenuItemType::Action:
            if (it.data.action.cb)
                it.data.action.cb(it.data.action.ctx);
            break;

        case MenuItemType::Int:
        case MenuItemType::Float:
        case MenuItemType::Bool:
            root.editState.editing = !root.editState.editing;
            break;
        }

        root.changed = true;
    }

    void adjustCurrent(MenuRoot &root, int increment)
    {
        if (!root.current || root.focusIndex >= root.current->itemCount) return;
        MenuItem &it = root.current->items[root.focusIndex];

        if (it.type == MenuItemType::Int && root.editState.editing && it.data.integer.value)
        {
            int v = *(it.data.integer.value);
            int step = it.data.integer.step * increment;
            v += step;
            v = constrain(v, it.data.integer.minVal, it.data.integer.maxVal);
            *(it.data.integer.value) = v;
            root.changed = true;
        }
        else if (it.type == MenuItemType::Float && root.editState.editing && it.data.floating.value)
        {
            float v = *(it.data.floating.value);
            float step = it.data.floating.step * increment;
            v += step;
            v = constrain(v, it.data.floating.minVal, it.data.floating.maxVal);
            *(it.data.floating.value) = v;
            root.changed = true;
        }
        else if (it.type == MenuItemType::Bool && root.editState.editing && it.data.boolean.value)
        {
            *(it.data.boolean.value) = !*(it.data.boolean.value);
            root.changed = true;
        }
        else
        {
            if (increment > 0)
                focusNext(root);
            else
                focusPrev(root);
        }
    }

} // namespace CompassMenu
