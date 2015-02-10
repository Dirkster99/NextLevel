function createjsDOMenu() {
  staticMenu1_1 = new jsDOMenu(130, "absolute");
  with (staticMenu1_1) {
    addMenuItem(new menuItem("Unreferenced", "LnF", "../../__unrefed_dbobj.html"));
    addMenuItem(new menuItem("Connections", "Con", "../../__ExternCalls.html"));
    addMenuItem(new menuItem("-", "", ""));

    addMenuItem(new menuItem("Help", "hlp", "index.html"));
    addMenuItem(new menuItem("About", "nl", "about.html"));
  }
  staticMenu1_1.items.LnF.showIcon("LostAndFound", "LostAndFound"); //Icons are defined here
  staticMenu1_1.items.Con.showIcon("Connections", "Connections");
  staticMenu1_1.items.nl.showIcon("nl", "nl");
  staticMenu1_1.items.hlp.showIcon("help", "help");

  absoluteMenuBar = new jsDOMenuBar("static", "staticMenu", false, "",200);
  with (absoluteMenuBar) {
    addMenuBarItem(new menuBarItem("Index", null,"idx",true,"../../index.html"));
    addMenuBarItem(new menuBarItem("Browse", staticMenu1_1,"s1"));
  }
}

