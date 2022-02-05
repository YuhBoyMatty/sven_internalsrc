# Sven Internal
Imagine cheat in a co-op game =)

Don't forget to get more hax at kekma !!

The code is a bit mess, basically didn't plan to publish it.

# Features
- Menu (key **INSERT** as default)
- ESP
- Wallhack
- Glow & Chams
- Vectorial Strafer
- Auto Bunnyhop
- Auto Jumpbug
- Fast Run
- Fake Lag
- Color Pulsator
- Speed Hack
- Anti-AFK
- Cam Hack
- Key Spammer
- First-Person Roaming
- [Advanced Mute System](https://github.com/sw1ft747/AdvancedMuteSystem "Advanced Mute System")
- Various Visual Hacks (Velometer, Crosshair, etc.)
- Message Spammer
- Skybox Replacement
- Custom Vote Popup
- Chat Colors

# Autoload config
Create file `sven_internal.cfg` in the following directory: `../Sven Co-op/svencoop/`.

And add this command in the file: `sc_load_config`.

If you load the cheat it will automatically execute file `sven_internal.cfg`, you can also use it for other purposes.

# Files of cheat
The cheat uses subfolder `sven_internal` in root directory of the game.

How it looks: `../Sven Co-op/sven_internal/`.

Note: some files/folders may not appear, you need to create them.

This folder is used to save the config, muted players, load players (their SteamID 64) for **Chat Colors** and load spam tasks for **Message Spammer**.

File `sven_internal.ini` is the config file (you can save it via menu or console command `sc_save_config`).

File `muted_players.db` automatically saved by the cheat when you exit from the game.

File `chat_colors_players.txt` allows to change chat color for a specific player.

Folder `message_spammer` is used by **Message Spammer** to load spam tasks.

Also, when cheat injected it will execute `sven_internal.cfg` file from folder `../Sven Co-op/svencoop/`.

# Console Variables/Commands
Type in the console the following command: `sc_help`.

The command above will print information about each CVar/ConCommand that belongs to the cheat.

# Chat Colors
Lets you change the color of players when they write something in chat.

File `chat_colors_players.txt` used for adding the players in such format: `STEAMID64 : COLOR_NUMBER`.

There're currently 5 color numbers (slots) that let you use 5 unique and customizable colors (can be change in Menu).

Example for the file:
```
76561198819023292 : 1 ; it's a comment.. (76561198819023292 : 1) <<< (STEAMID64 : COLOR_NUMBER)
```

The file automatically loads when cheat loaded. Also, you can use a console command `sc_chat_colors_load_players` to reload the players list.

# Message Spammer
Roughly, it's some kind of AHK.

Uses `*.txt` files from folder `message_spammer` to run spam tasks.

It supports 3 keywords: `loop`, `send` and `sleep`.

Important: Message Spammer reads the `*.txt` files sequentially. Also, use the following console command to run a spam task: `sc_ms_add [FILENAME]`.

For example:
```
sleep 1.5
send test string
```
It will be executed like: wait 1.5 seconds, then send to game chat `test string`

### loop
Must be placed at the beginning of the file.

Loops the spam task, otherwise if you don't place that keyword in your .txt file, then the spam task will be executed once.

### send [message]
Sends a message to game chat.

One argument: [message].

### sleep [delay]
Sleeps a spam task.

One argument: [delay].

**Example:**
```
loop
send stuck
sleep 0.35
send play
sleep 0.35
send diff
sleep 0.35
send alone
sleep 0.35
send light
sleep 0.35
send my ears
sleep 0.35
send vote
sleep 0.35
send buy
sleep 0.35
send thief
sleep 125.0
```
