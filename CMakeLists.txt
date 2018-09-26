cmake_minimum_required(VERSION 3.10)
project(flyDB)

set(CMAKE_CXX_STANDARD 11)

add_executable(flyDB main.cpp server/FlyServer.cpp server/FlyServer.h server/FlyDB.cpp server/FlyDB.h server/CommandEntry.cpp server/CommandEntry.h server/dataStructure/dict/Dict.cpp server/dataStructure/dict/Dict.h server/dataStructure/dict/HashTable.cpp server/dataStructure/dict/HashTable.h server/dataStructure/dict/DictEntry.h server/dataStructure/dict/DictEntry.cpp server/dataStructure/dict/test/TestDictType.cpp server/dataStructure/dict/test/TestDictType.h server/dataStructure/dict/DictDef.h server/dataStructure/skiplist/SkipListNode.cpp server/dataStructure/skiplist/SkipListNode.h server/dataStructure/skiplist/SkipListType.h server/dataStructure/dict/DictType.h server/dataStructure/skiplist/SkipList.cpp server/dataStructure/skiplist/SkipList.h server/dataStructure/skiplist/SkipListDef.h server/dataStructure/skiplist/test/TestSkipListType.cpp server/dataStructure/skiplist/test/TestSkipListType.h)