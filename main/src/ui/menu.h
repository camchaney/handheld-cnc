#ifndef menu_h
#define menu_h
#include <cstdint>

namespace CompassMenu {
  struct MenuEditState {
    bool editing = false;
  };

  typedef void (*MenuCallback)(void* ctx);

  enum class MenuItemRenderType : uint8_t {
    Text,
    WithValue,
    Drawing
  };

  enum class MenuItemType : uint8_t {
    Submenu,
    Back,
    Action,
    Bool,
    Int,
    Float
  };

  struct Menu;

  struct MenuItem {
    const char* label;
    MenuItemType type;
    union {
      struct { Menu* ptr; } submenu;
      struct { /* empty */ } back;
      struct { MenuCallback cb; void* ctx; } action;
      struct { bool* value; bool wrap; } boolean;
      struct { int* value; int minVal; int maxVal; int step; const char* unitOfMeasure; } integer;
      struct { float* value; float minVal; float maxVal; float step; const char* unitOfMeasure; } floating;
    } data;
  };

  typedef void (*MenuDrawCallback)(MenuItem* menuItem);

  struct Menu {
    const char* title;
    Menu* parent;
    MenuItem* calledFrom;
    MenuItem* items;
    uint8_t itemCount;
    MenuItemRenderType renderType;
    MenuDrawCallback drawCallback;
  };

  struct MenuRoot {
    Menu* top;
    Menu* current;
    uint8_t focusIndex;
    MenuEditState editState;
    bool changed;
  };

  MenuItem makeSubmenu(const char* label, Menu* submenu);
  MenuItem makeBack(const char* label = "Back");
  MenuItem makeAction(const char* label, MenuCallback cb, void* ctx = nullptr);
  MenuItem makeBool(const char* label, bool* value, bool wrap = true);
  MenuItem makeInt(const char* label, int* value, int minVal, int maxVal, int step = 1, const char* unitOfMeasure = nullptr);
  MenuItem makeFloat(const char* label, float* value, float minVal, float maxVal, float step = 1, const char* unitOfMeasure = nullptr);

  void focusNext(MenuRoot& root);
  void focusPrev(MenuRoot& root);
  void back(MenuRoot& root);
  void enter(MenuRoot& root);
  void adjustCurrent(MenuRoot& root, int increment);
}
#endif // menu_h