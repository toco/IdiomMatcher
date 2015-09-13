#IdiomMatcher
Part of my bachelor thesis 'Matching of Control- and Data-Flow Constructs in Disassembled Code'.
The goal is to implement a plugin for [IDA](https://www.hex-rays.com/products/ida/index.shtml) which allows to find idioms specified by a pattern and mark them accordingly in IDA.
This aids the reverse engineering of programs for unusual architectures, for which not all constructs are detected correctly. 

##Dependencies
- [IDA SDK](https://www.hex-rays.com/products/ida/index.shtml), currently using version 6.7
- [Boost](http://www.boost.org), currently using 1.58.0
- [RapidJSON](https://github.com/miloyip/rapidjson/), currently using 1.0.2

##Compile
- Download the dependencies and place them in a directory ```contrib```.
- create a build directory, e.g. ```build```
- ```cd``` in the build directory.
- run ```cmake``` with the following options, values may be depending on your configuration.

```
cmake -D IDA_DIR="/Applications/idaq.app/Contents/MacOS" -D IDA_SDK="contrib/idasdk67/" -D BOOST_DIR="contrib/boost_1_58_0" -D RAPID_JSON_DIR="contrib/rapidjson/include/" install ../
```

If you compile and install using cmake, it will automatically place the IdiomMatcher plugin in the plugin directory of IDA.

##Pattern Specification.
Have a look into the patterns in the directory ```eval``` for examples of pattern specification.
Normally you wouldn't manually write patterns, instead select some disassembly in IDA and create a pattern from there.
Select disassembly and use the context menu or the menu "Edit/Other/Create Pattern from selection" 

For more information about the pattern specification have a look at the documented IdiomMathcher.json example in the ```doc``` directory.

##Plugin usage
Load patterns form disk or create patterns 

##TODO
- Actually use extracted values as parameter in match action.
- Add GUI for pattern editing.

##License
Licenced under MIT License, see LICENSE for full text.