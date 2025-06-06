## Project: Peer-to-Peer Tic-Tac-Toe Game (C Implementation)

### Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [Protocol Overview](#protocol-overview)

   * [P2P Architecture](#p2p-architecture)
   * [Roles: Master and Player](#roles-master-and-player)
4. [Command Reference](#command-reference)

   * [Master Commands](#master-commands)
   * [Player Commands](#player-commands)
   * [Error Commands](#error-commands)
5. [State Machines](#state-machines)

   * [Master’s Automaton](#masters-automaton)
   * [Player’s Automaton](#players-automaton)
7. [Compile and Run](#compile-and-run)
8. [Usage / Game Flow](#usage--game-flow)

---

## Introduction

This repository contains a C-based implementation of a decentralized, peer-to-peer (P2P) Tic-Tac-Toe game for two players. Rather than relying on a centralized server, each peer (player) and a “master” node communicate directly over the network, enabling a fully distributed game session. The master node manages player registration, matchmaking, and scorekeeping, while individual game sessions occur directly between peers.

*Adapted from project report (“INFORME DE IMPLEMENTACIÓN DE UN JUEGO PEER - 2-PEER”) by Luis Fernando Zapata Moya & Natalia Delgado Soruco (2024).*

---

## Features

* **Decentralized P2P Communication**
  The game logic runs in a fully decentralized fashion. Players connect to a master node but exchange moves directly, minimizing latency and avoiding a single point of failure.

* **Master Node Responsibilities**

  * Player registration (stores alias, scores)
  * Matchmaking: provides list of available opponents to players
  * Scorekeeping: tracks wins, losses, draws; maintains a scoreboard for up to 10 concurrent connected players.

* **Peer-to-Peer Match Sessions**
  Once a game is initiated, the two selected players bypass the master and exchange moves directly until game completion.

* **Command-Based Protocol**
  All communication (registration, matchmaking, move exchange, result reporting) is text-based, using well-defined commands.

---

## Protocol Overview

### P2P Architecture

* **Decentralization**
  Each node (master or player) can act as a client or server at different phases of the game. No single server hosts game state beyond initial matchmaking and score reporting.
* **Auto-Organization**
  Players discover and connect to the master to register. When two players agree to a match, the master “introduces” them by sending IP/port information, and the game proceeds P2P.
* **Scalability & Robustness**
  Up to 10 players can be connected to the master concurrently. Once a match is in progress, neither player relies on the master until the session ends, limiting the master’s load to one role at a time.

---

### Roles: Master and Player

* **Master Node**

  1. Listens on a known port; accepts incoming TCP connections from players.
  2. Receives registration requests (alias).
  3. Maintains an in-memory table of connected players, their scores (wins, losses, draws).
  4. Upon a “request game” command, provides a list of available opponents.
  5. After opponents are selected, sends the challengers each other’s IP/port to establish a direct P2P session.
  6. Receives a “FINN” message from the winning or losing peer to update the scoreboard.

* **Player Node**

  1. Connects to the master to register (providing alias).
  2. Requests the list of available players when wanting to start a match.
  3. Sends a challenge request to the master, specifying chosen opponent’s ID and its own listening port.
  4. If match request is accepted, establishes a direct TCP connection to the opponent for the game session.
  5. Alternates sending/receiving moves until the game concludes, then reports the result (“W/L/D”) back to the master.

---

## Command Reference

Below are the text-based commands exchanged between master and players. Each command is sent as a line of ASCII text, with fields separated by pipe (`|`) or angle-bracket (`<`) delimiters.

### Master Commands

| Code | Name                   | Format                                                                              | Explanation                                                                                           |
|:----:|------------------------|-------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------|
| 100  | **READY**              | (no payload)                                                                        | Master acknowledges it is ready to receive.                                                           |
| 101  | **RECEIVED**           | (no payload)                                                                        | General acknowledgment that a message was received.                                                   |
| 102  | **PLAYER_REGISTERED**  | (no payload)                                                                        | Confirms that a player’s registration was successful.                                                 |
| 103  | **GAME_REGISTERED**    | (no payload)                                                                        | Confirms a game request has been logged.                                                              |
| 104  | **STATE_PLAYING**      | (no payload)                                                                        | Informs master changed to “playing” status.                                                           |
| 105  | **STATE_READY**        | (no payload)                                                                        | Informs master changed to “ready” status.                                                             |
| 200  | **PLAYER_INFO**        | `200\|<id><alias><points><wins><losses><draws>`                                     | Sends a single player’s stats: ID, alias, total points (win=1, draw=0.5, loss=0), number of wins, losses, draws. |
| 201  | **ALL_PLAYERS**        | `201\|<id><alias><points><wins><losses><draws>...`                                  | Sends a list of all connected players with their stats.                                               |
| 202  | **PLAYERS_AVAILABLE**  | `202\|<id><alias>...`                                                                | Sends a list of players who are not currently in a match.                                             |
| 203  | **SEND_CHALLENGE**     | `203\|<id>\|<alias>\|<ip>\|<listening_port>`                                           | Instructs a player to initiate a direct connection to another player.                                 |
| 204  | **CHALLENGE_RESPONSE** | `204\|<Y/N>`                                                                          | Informs master whether a direct challenge succeeded (Y) or failed (N).                                |

---

### Error Commands

| Code | Name                                | Explanation                                                     |
| :--: | ----------------------------------- | --------------------------------------------------------------- |
|  400 | **SERVER\_NOT\_VALIDATED**          | Connection to server (master) failed authorization/handshake.   |
|  401 | **PLAYER\_NOT\_REGISTERED**         | A player attempted an action before registering.                |
|  402 | **INVALID\_ALIAS**                  | Alias provided during registration is invalid or already taken. |
|  403 | **SERVER\_NOT\_READY**              | Master is not in a “ready” state to accept requests.            |
|  404 | **MESSAGE\_CONFIRMATION\_REQUIRED** | A message was incomplete—confirmation part missing.             |
|  405 | **NO\_VALID\_CHALLENGE\_RESPONSE**  | Opponent did not respond to a challenge.                        |
|  406 | **INVALID\_PLAY\_MODE\_RESPONSE**   | Player responded with invalid data during “play” negotiation.   |
|  407 | **CHALLENGE\_RESULT\_INVALID**      | The reported match result does not match the game state.        |
|  420 | **INVALID\_COMMAND**                | Received command is not recognized.                             |

---

### Player Commands

|  Command | Format                     | Explanation                                                                                           | 
| :------: | -------------------------- | ----------------------------------------------------------------------------------------------------- | 
| **REGI** | `REGI<"ALIAS">`            | Register player alias with master.                                                                    |                                                                        
| **LIST** | (no payload)               | Request to master for list of all players and their scores.                                           |                                                                        
| **OKAY** | (no payload)               | Acknowledgment of correctly received data.                                                            |                                                                        
| **DESA** | `DESA<id><listening_port>` | Request from player to master to challenge another player (by ID), specifying its own listening port. |                                                                       
| **RESP** | `RESP<id><Y/N>`            | Player’s response to a challenge—`Y` to accept or `N` to decline.                                     |                                                                        
| **HOLA** | (no payload)               | “Hello” handshake message to indicate readiness.                                                      |                                                                        
| **JUGA** | (no payload)               | Request to master to initiate a new game.                                                             |                                                                        
| **POSX** | `POSX\| <XN>\`            | Player move, where `XN` is the position (1–9) on the 3×3 board.                                       |
| **FINN** | `FINN\| \<W/L/D>\`        | End-of-game notification to master: `W` (win), `L` (loss), `D` (draw). |
| **WINN** | (no payload)               | Notification directly to opponent that the game has ended (both sides send).                          |                                                                        |
| **INFO** | (no payload)               | Request for own player info from master.                                                              |                                                                        |
| **PLYE** | (no payload)               | Notification that “play” request was accepted by opponent.                                            |                                                                        |
| **PLNO** | (no payload)               | Notification that “play” request was rejected by opponent.                                            |                                                                        |

---

## State Machines

### Master’s Automaton

The master transitions through these states when interacting with players:

1. **F (Free/Unoccupied)**

   * Initially, master is in “Free” state: no players connected.
   * Waits for new TCP connections from players.

2. **N (Not Registered)**

   * When a new TCP connection arrives, player enters “N” until registration is completed.
   * Master expects the `REGI<"ALIAS">` command from the player.

3. **R (Ready)**

   * After successful registration (`102 PLAYER_REGISTERED`), master marks this player as “Ready.”
   * Player is idle and can request to view scoreboard or challenge another player.

4. **W (Waiting for Confirmation)**

   * When master sends information (e.g., available players) to the player, it waits for `OKAY` acknowledgment.

5. **P (Playing)**

   * Once a match is arranged, master changes the state to “Playing” for both participants (removing them from the “Ready” pool).
   * Master no longer mediates direct game messages; waits for final result (`FINN|<W/L/D>`) from a peer.



![MasterAutomaton](/Images/MasterAutomaton.png)

---

### Player’s Automaton

A player’s interaction states are:

1. **R (Ready)**

   * After sending `REGI<"ALIAS">` and receiving confirmation, the player is “Ready.”
   * Can request `LIST` or initiate `DESA<id><port>` to challenge another.

2. **W (Waiting for Master’s Response)**

   * After sending a command (e.g., `DESA`), the player waits for a response from master (`203 SEND_CHALLENGE` or an error code).

3. **A (Pre-Game / Awaiting Challenge Response)**

   * After receiving a challenge from master (`203|id|alias|ip|port`), the player enters “A” to decide:

     * Send `RESP<id><Y>` to accept, or
     * Send `RESP<id><N>` to decline.

4. **P (Playing)**

   * Once both players have exchanged listening ports (via master coordination), they connect directly.
   * Alternate sending `POSX|<position>` messages until game ends.
   * At game end, both send `WINN` to each other and the winner/loser sends `FINN|<W/L>` to master.


![PlayerAutomaton](/Images/PlayerAutomaton.png)
---


## Compile and Run

### Prerequisites

* GCC (or any C99-compatible compiler)
* `make` (GNU Make)
* POSIX-compliant OS (e.g., Linux, macOS)

### Building

1. Clone the repository:

   ```bash
   git clone https://github.com/LuisZapata22/P2PTicTacToe
   cd P2PTicTacToe
   ```

2. Build both the master and player executables:

   ```bash
   make
   ```

   The `Makefile` compiles:

   * `master/master.c` → `master`
   * `player/player.c` → `player`


---

### Running

1. **Start the Master Node**
   On one terminal (the “host” machine), launch the master. By default, the master listens on port `2024`. You can override via a command-line argument.

   ```bash
   # Default port 2024
   ./master
   ```

   Output example:

   ```
   [MASTER] Listening on port 5000...
   ```

2. **Start Player Nodes**
   On separate terminals (or separate machines on the same LAN), launch each player. Each player must specify:

   * IP address (or hostname) of the master node
   * Master’s listening port
   * Local listening port (for P2P game sessions)

   ```bash
   ./bin/player <master_ip> <master_port> 
   ```

   Example (master on `192.168.1.10`, port `5000`):

   ```bash
   ./bin/player 192.168.1.10 5000 6000
   ```

   Output example:

   ```
   [PLAYER] Enter your alias:
   ```

3. **Gameplay Flow**

   * **Registration**

     * Player enters a unique alias (e.g., `Alice`).
     * Master acknowledges registration (`102 PLAYER_REGISTERED`).
   * **Viewing Available Players / Scoreboard**

     * Player types `LIST` → Master responds with code `201 ALL_PLAYERS` (with stats) or `202 PLAYERS_AVAILABLE` (only idle players).
   * **Initiating a Match**

     * Player A sends `DESA<id_of_Player_B><local_listen_port>`.
     * Master verifies that Player B is available and issues `203 SEND_CHALLENGE` to Player B containing Player A’s alias, IP, and port.
     * Player B receives the challenge and chooses `RESP<id_of_Player_A><Y>` to accept (or `<N>` to decline).
     * If accepted, both players switch to “Playing” state, open a direct TCP connection:

       * Player A connects to Player B at `<Player_B_IP>:<Player_B_listen_port>`.
       * Player B already has a listening socket bound on its local listen port.
   * **Playing Tic-Tac-Toe**

     * Players alternate sending `POSX|<position>` commands (positions `1–9` map to the 3×3 grid).
     * Each peer maintains its local copy of the board; after each move, the receiver updates its board and sends an acknowledgment (`OKAY`).
     * After a win, loss, or draw, both peers send `WINN` to each other, then the winner (or losing peer) sends `FINN|<W/L/D>` to the master.
   * **Scoreboard Update**

     * Master receives `FINN|<result>` from each player (should match; if conflicting, error code `407` is returned).
     * Master updates the alias’s win/loss/draw counters and recalculates total points (`win = 1`, `draw = 0.5`, `loss = 0`).
   * **Return to Lobby**

     * Both peers revert to state “Ready,” and can either issue `LIST` to view the updated scoreboard or `DESA` to start a new match.

