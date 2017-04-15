Console Cabby
=============

```txt
+--------------------------------------------------+ CONSOLE CABBIE
|  ######\ ######\ ######\ ######\ ######\  #######|
|  \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\  #######| TIME:  474
| C                     C                  C#######| MONEY: 0
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\C######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######| WASD to move, Q to quit.
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ $#####\ ######\ ######\ C#######| Pick up fares($) and drop them off
|  \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\  #######| at the destination(X) before
|         C    C          @   C          C  #######| time runs out.
|  ######\ ######\C######\ ######\C#$####\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  ######\ ######\ ######\ ######\ ######\  #######|
|  \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\ \_\_\_\  #######|
|                                           #######|
|C               C        C                 #######|
+--------------------------------------------------+
```

A simple terminal based game where you drive around and pick up fares to collect money before time runs out. This game was 
written in C++ and uses the great [rlutil](https://github.com/tapio/rlutil/) library as well as a few common C/C++ libs.

The only true dependency is rlutil which comes with the code so you shouldn't need to install anything to get it compiled 
and running.

## HOW TO PLAY ##
Use the WASD keys to drive around the map. Moving next to a fare, represented by `$`, will pick them up. Then an `X` will appear on the map. Move next to the `X` to drop the fare off and collect some money. Try to collect as much money as possible before time runs out. 

## COMPILE ##
To compile console cabby use the supplied make file then run the `cabby` binary. 

```sh
$ make
$ ./cabby
```

## OPTIONS ##
There are several options you can pass to adjust the game. 

 * `-cc <int>` Car count. Adjusts the number of cars on the road.
 * `-fc <int>` Fare count. Adjust the number of available fares on the map at any one time.
 * `-h` Help. Prints out the available options.
 * `-m <file path>` Map. You can import your own maps if you'd like to play on a different layout than what's available. Check below for how to format map files. 
 * `-r` Reverse. Makes traffic drive on the left side of the road (U.K. style).
 * `-t <int>` Time. Adjust the starting time.
 
## MAKING NEW MAPS ##
Map files are just text files that use the characters `#1245689:`. Every row of text in the file should be the same length. The `#` represents a wall or non-traversable tile and the numbers represent which direction traffic can move in on that tile. 

#### Movement Values ####
 * `1` Means a car can only travel UP.
 * `2` Means a car can only travel DOWN.
 * `4` Means a car can only travel RIGHT.
 * `5` Means a car can travel either UP or RIGHT.
 * `6` Means a car can travel either DOWN or RIGHT.
 * `8` Means a car can only travel LEFT.
 * `9` Means a car can travel either UP or LEFT.
 * `:` Means a car can travel either DOWN or LEFT.
 
#### Example ####
Here's an example with a two way outer ring and two one way cross streets.
```
##############################
#:888888888888888888888888889#
#2544444444444444444444444461#
#21########################21#
#21########################21#
#21########################21#
#29888888888888888888888888:1#
#21########################21#
#21########################21#
#21########################21#
#2544444444444444444444444461#
#21########################21#
#21########################21#
#21########################21#
#29888888888888888888888888:1#
#6444444444444444444444444445#
##############################
```
_Note_: You don't have to ring the outside of your map with walls, but it will make your map render prettier. 


## LICENSE ##
MIT Licensed

_Copyright (C) 2017 Jim Saunders_
