#include "TextAdventureWidget.hpp"
#include "TutorialDialog.hpp"

TextAdventureWidget::TextAdventureWidget(QWidget *parent)
    : QWidget(parent)
{
    // MAIN HORIZONTAL LAYOUT
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // LEFT SIDE (game output + input)
    QVBoxLayout *leftLayout = new QVBoxLayout;

    output = new QTextEdit(this);
    output->setReadOnly(true);

    input = new QLineEdit(this);
    input->setPlaceholderText("Enter a command...");

    QPushButton *submit = new QPushButton("Go", this);

    leftLayout->addWidget(output);
    leftLayout->addWidget(input);
    leftLayout->addWidget(submit);

    connect(submit, &QPushButton::clicked, this, &TextAdventureWidget::onCommandEntered);
    connect(input, &QLineEdit::returnPressed, this, &TextAdventureWidget::onCommandEntered);

    // RIGHT SIDE (minimap)
    minimap = new MinimapWidget(this);
    minimap->setMinimumSize(400, 200);

    // Add both sides to main layout
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addWidget(minimap, 1);

    // Build world AFTER UI is ready
    setupWorld();
    showRoom("cabin");

    // Tutorial popup
    TutorialDialog tutorial(this);
    //tutorial.exec();
}


void TextAdventureWidget::setupWorld() {
    // --- Cabin ---
    Room cabin;
    cabin.description = "You are inside a small wooden cabin. A storm rages outside.";
    cabin.exits["outside"] = "forest";
    cabin.exits["path"] = "path";

    // --- Forest ---
    Room forest;
    forest.description = "Tall trees surround you. The wind howls through the branches.";
    forest.exits["south"] = "cabin";
    forest.exits["north"] = "clearing";
    forest.exits["east"] = "riverbank";
    forest.exits["west"] = "cave";

    // --- Clearing ---
    Room clearing;
    clearing.description = "A quiet clearing with soft grass. The storm feels distant here.";
    clearing.exits["south"] = "forest";
    clearing.exits["north"] = "rocky_trail";

    // --- Rocky Trail ---
    Room rockyTrail;
    rockyTrail.description = "A steep rocky trail climbs toward a distant mountain peak.";
    rockyTrail.exits["south"] = "clearing";
    rockyTrail.exits["north"] = "mountain_peak";

    // --- Mountain Peak ---
    Room mountainPeak;
    mountainPeak.description = "You stand atop a jagged peak. The world stretches out below.";
    mountainPeak.exits["south"] = "rocky_trail";

    // --- Cave ---
    Room cave;
    cave.description = "A dark cave mouth yawns before you. Cold air drifts out.";
    cave.exits["east"] = "forest";
    cave.exits["down"] = "underground_lake";
     cave.lockedExits["down"] = "lantern";              // Need lantern to descend safely
    // --- Underground Lake ---
    Room undergroundLake;
    undergroundLake.description = "A vast underground lake glows faintly with blue light.";
    undergroundLake.exits["up"] = "cave";

    // --- Riverbank ---
    Room riverbank;
    riverbank.description = "A fast-moving river blocks your path. Mist rises from the water.";
    riverbank.exits["west"] = "forest";
    riverbank.exits["east"] = "old_bridge";

    // --- Old Bridge ---
    Room oldBridge;
    oldBridge.description = "An ancient wooden bridge creaks under your weight.";
    oldBridge.exits["west"] = "riverbank";
    oldBridge.exits["east"] = "ruins_entrance";

    // --- Ruins Entrance ---
    Room ruinsEntrance;
    ruinsEntrance.description = "Crumbling stone pillars mark the entrance to forgotten ruins.";
    ruinsEntrance.exits["west"] = "old_bridge";
    ruinsEntrance.exits["down"] = "deep_ruins";
    ruinsEntrance.lockedExits["down"] = "rusty_key";   // Need key to enter deep ruins
    // --- Deep Ruins ---
    Room deepRuins;
    deepRuins.description = "Dusty halls stretch into darkness. Something stirs in the shadows.";
    deepRuins.exits["up"] = "ruins_entrance";

    // --- Path ---
    Room path;
    path.description = "A muddy path leads away from the cabin toward a distant village.";
    path.exits["west"] = "cabin";
    path.exits["east"] = "village_gate";

    // --- Village Gate ---
    Room villageGate;
    villageGate.description = "A wooden gate stands open. Lanterns flicker in the rain.";
    villageGate.exits["west"] = "path";
    villageGate.exits["east"] = "village_square";   
    villageGate.lockedExits["east"] = "village_pass";  // Puzzle reward

    // --- Village Square ---
    Room villageSquare;
    villageSquare.description = "A quiet village square with shuttered windows and empty streets.";
    villageSquare.exits["west"] = "village_gate";

    itemDescriptions["lantern"] = "A small oil lantern. The glass is cracked but usable.";
    itemDescriptions["stick"] = "A sturdy wooden stick. Could be useful as a makeshift weapon.";
    itemDescriptions["herbs"] = "A bundle of fragrant forest herbs. They smell calming.";
    itemDescriptions["old_coin"] = "A tarnished coin with strange markings. Very old.";
    itemDescriptions["glowing_mushroom"] = "A faintly glowing mushroom. Emits a soft blue light.";
    itemDescriptions["rusty_key"] = "A rusted iron key. It looks ancient.";
    itemDescriptions["ancient_scroll"] = "A brittle scroll covered in faded runes.";
    itemDescriptions["apple"] = "A fresh red apple. Looks delicious.";

    cabin.items << "lantern";
    forest.items << "stick";
    clearing.items << "herbs";
    cave.items << "old_coin";
    undergroundLake.items << "glowing_mushroom";
    ruinsEntrance.items << "rusty_key";
    deepRuins.items << "ancient_scroll";
    villageSquare.items << "apple";

    // Register rooms
    rooms["cabin"] = cabin;
    rooms["forest"] = forest;
    rooms["clearing"] = clearing;
    rooms["rocky_trail"] = rockyTrail;
    rooms["mountain_peak"] = mountainPeak;
    rooms["cave"] = cave;
    rooms["underground_lake"] = undergroundLake;
    rooms["riverbank"] = riverbank;
    rooms["old_bridge"] = oldBridge;
    rooms["ruins_entrance"] = ruinsEntrance;
    rooms["deep_ruins"] = deepRuins;
    rooms["path"] = path;
    rooms["village_gate"] = villageGate;
    rooms["village_square"] = villageSquare;

    currentRoom = "cabin";
    minimap->discoverRoom(currentRoom);   // only discover the starting room
minimap->setCurrentRoom(currentRoom);
}


void TextAdventureWidget::showRoom(const QString &roomName) {
    const Room &r = rooms[roomName];
    minimap->setCurrentRoom(roomName);
    output->append("--------------------------------------------------");
    output->append(r.description);

    // Show items
    if (!r.items.isEmpty()) {
        QString itemLine = "You see here: ";
        for (const QString &item : r.items)
            itemLine += item + " ";
        output->append(itemLine);
    }

    if (roomName == "clearing" && !puzzleSolved["clearing_riddle"]) {
    output->append("Something feels strange here. Perhaps there is a riddle to solve.");
}

    // Show exits
    QString exits = "Exits: ";
    for (auto it = r.exits.begin(); it != r.exits.end(); ++it)
        exits += it.key() + " ";
    output->append(exits);

    if (!r.items.isEmpty()) {
    QString itemLine = "You see here: ";
    for (const QString &item : r.items)
        itemLine += item + " ";
    output->append(itemLine);
    output->append("You can 'examine <item>' to learn more.");
}
}


void TextAdventureWidget::onCommandEntered() {
    QString cmd = input->text().trimmed();
    input->clear();

    if (cmd.isEmpty()) return;

    processCommand(cmd);
}

void TextAdventureWidget::processCommand(const QString &cmd) {
    
    QString lower = cmd.toLower();
// --- Normalize movement commands ---
QString dir = lower;

// Handle "go north", "go n", etc.
if (dir.startsWith("go ")) {
    dir = dir.mid(3).trimmed();
}

// Shortcuts
if (dir == "n") dir = "north";
if (dir == "s") dir = "south";
if (dir == "e") dir = "east";
if (dir == "w") dir = "west";
if (dir == "u") dir = "up";
if (dir == "d") dir = "down";

// --- Movement ---
if (rooms[currentRoom].exits.contains(dir)) {

    // Check locked exits
    if (rooms[currentRoom].lockedExits.contains(dir)) {
        QString requiredItem = rooms[currentRoom].lockedExits[dir];

        if (!inventory.contains(requiredItem)) {
            output->append("The way " + dir + " is locked. You need a " + requiredItem + ".");
            return;
        }

        output->append("You use the " + requiredItem + " to unlock the way " + dir + ".");
    }

    currentRoom = rooms[currentRoom].exits[dir];
    showRoom(currentRoom);
    return;
}

    if (lower == "help") {
    TutorialDialog tutorial(this);
    tutorial.exec();
    return;
}
    if (lower == "look") {
        showRoom(currentRoom);
        return;
    }

    // --- Solve puzzle ---
if (lower.startsWith("solve ")) {
    QString puzzle = lower.mid(6).trimmed();

    if (puzzle == "riddle" && currentRoom == "clearing") {
        output->append("The wind whispers: 'I speak without a mouth and hear without ears. What am I?'");
        output->append("Type: answer echo");
        return;
    }

    output->append("There is no puzzle like that here.");
    return;
}

// --- Puzzle answers ---
if (lower.startsWith("answer ")) {
    QString ans = lower.mid(7).trimmed();

    if (currentRoom == "clearing" && ans == "echo") {
        output->append("The forest quiets. A hidden compartment opens, revealing a village_pass.");
        rooms["clearing"].items.append("village_pass");
        puzzleSolved["clearing_riddle"] = true;
        return;
    }

    output->append("That doesn't seem right.");
    return;
}

    // --- Movement ---
if (rooms[currentRoom].exits.contains(lower)) {

    // Check if exit is locked
    if (rooms[currentRoom].lockedExits.contains(lower)) {
        QString requiredItem = rooms[currentRoom].lockedExits[lower];

        if (!inventory.contains(requiredItem)) {
            output->append("The way " + lower + " is locked. You need a " + requiredItem + ".");
            return;
        }

        output->append("You use the " + requiredItem + " to unlock the way " + lower + ".");
    }

    currentRoom = rooms[currentRoom].exits[lower];
    showRoom(currentRoom);
    return;
}
    // --- Inventory ---
if (lower == "inventory" || lower == "i") {
    if (inventory.isEmpty()) {
        output->append("You are carrying nothing.");
    } else {
        output->append("You are carrying:");
        for (const QString &item : inventory)
            output->append(" - " + item);
    }
    return;
}

// --- Take item ---
if (lower.startsWith("take ")) {
    QString item = lower.mid(5).trimmed();

    if (rooms[currentRoom].items.contains(item)) {
        rooms[currentRoom].items.removeAll(item);
        inventory.append(item);
        output->append("You take the " + item + ".");
    } else {
        output->append("There is no " + item + " here.");
    }
    return;
}

// --- Drop item ---
if (lower.startsWith("drop ")) {
    QString item = lower.mid(5).trimmed();

    if (inventory.contains(item)) {
        inventory.removeAll(item);
        rooms[currentRoom].items.append(item);
        output->append("You drop the " + item + ".");
    } else {
        output->append("You don't have a " + item + ".");
    }
    return;
}

// --- Examine item ---
if (lower.startsWith("examine ") || lower.startsWith("inspect ") ||
    lower.startsWith("look at ") || lower.startsWith("x ")) {

    QString item;

    if (lower.startsWith("examine "))
        item = lower.mid(8).trimmed();
    else if (lower.startsWith("inspect "))
        item = lower.mid(8).trimmed();
    else if (lower.startsWith("look at "))
        item = lower.mid(8).trimmed();
    else if (lower.startsWith("x "))
        item = lower.mid(2).trimmed();

    // Check inventory first
    if (inventory.contains(item)) {
        if (itemDescriptions.contains(item))
            output->append(itemDescriptions[item]);
        else
            output->append("You see nothing special about the " + item + ".");
        return;
    }

    // Check room items
    if (rooms[currentRoom].items.contains(item)) {
        if (itemDescriptions.contains(item))
            output->append(itemDescriptions[item]);
        else
            output->append("You see nothing special about the " + item + ".");
        return;
    }

    output->append("You don't see that here.");
    return;
}

// --- Solve puzzle ---
if (lower.startsWith("solve ")) {
    QString puzzle = lower.mid(6).trimmed();

    if (puzzle == "riddle" && currentRoom == "clearing") {
        output->append("The wind whispers: 'I speak without a mouth and hear without ears. What am I?'");
        output->append("Type: answer echo");
        return;
    }

    output->append("There is no puzzle like that here.");
    return;
}

// --- Puzzle answers ---
if (lower.startsWith("answer ")) {
    QString ans = lower.mid(7).trimmed();

    if (currentRoom == "clearing" && ans == "echo") {
        output->append("The forest quiets. A hidden compartment opens, revealing a village_pass.");
        rooms["clearing"].items.append("village_pass");
        puzzleSolved["clearing_riddle"] = true;
        return;
    }

    output->append("That doesn't seem right.");
    return;
}

    output->append("You can't do that.");
}
