## The base module for pingworld

### contents

1. log library;
2. message queue for communicating between threads;
3. redis client;

### modules

1. 3rd
	The dependance of all other libs.

2. b1lib
	The module called libb1 which will be the common lib for all other libs.
	- base64
	- log
	- mq
	- redis client
	- b1string, which is std::vector<char>

3. b2lib
	将Chromium的base库改造成我自己合用的库，具体的说明参考目录中的readme.md

3. makefiles
	我在开发中常用的makefile模板
	- CMakeFiles.txt.tpl

### changelog

1. 2016.5.26 
	Fork from pingworld. Devide the base lib from it.

