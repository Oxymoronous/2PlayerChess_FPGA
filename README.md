# 2PlayerChess_FPGA
UIUC ECE385 Solo Final Project

Disclaimer:
This directory exists mainly as a purpose for new programmers who want to have a general design idea on FPGA chess programming.
There is no concise step-by-step instructions how to take this codebase and set it up onto a FPGA board.

Description:
This is my first, solo SoC project where I designed and implemented a 2-player chess game on TERASIC DE10-lite as part of UIUC ECE385 final project.
The game is meant to be displayed on a VGA monitor.
The chess game is controlled by a mouse, with a simple sidebar interface to display game information and controls.

The game logic is all done in the C software files, whereas the sprite drawing logic on the VGA monitor is handled by HDL. Some more detailed explanation of the architectural design and a high level block diagram can be found in the Report PDF in one of the directories.
