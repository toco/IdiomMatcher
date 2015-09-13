//
// Created by Tobias Conradi on 13.09.15.
// Licensed under MIT License, see LICENSE for full text.


#include <getopt.h>
#include <sysexits.h>
#include <Standalone/IdiomMatcherStandalone.h>
#include <Model/Logging.h>

void mylogging(const char *format, ...);
void printUsage(char *name);
bool parseArgumens(IdiomMatcherStandalone &standalone, int argc, char *argv[]);

int main(int argc, char* argv[]) {

    IdiomMatcher::warning = mylogging;
    IdiomMatcher::info = mylogging;
    IdiomMatcher::msg = printf;

    IdiomMatcherStandalone matcher;
    bool parsingSuccess = parseArgumens(matcher, argc, argv);
    if (!parsingSuccess) {
        printUsage(argv[0]);
        return EX_USAGE;
    }

    auto api = matcher.readDisassembly();

    if (matcher.shouldDumpSwitches) {
        matcher.dumpSwitches(api);
        return EXIT_SUCCESS;
    }

    if (!matcher.readPatterns()) {
        exit(EX_DATAERR);
    }

    matcher.matchAll(api);
    return EXIT_SUCCESS;
}

void mylogging(const char *format, ...) {
    va_list vaargs;
    va_start(vaargs, format);
    vprintf(format, vaargs);
    va_end(vaargs);
}

void printUsage(char *name) {
    printf("usage: %s --file DisassemblyFilePath.json --patterns PatternFilePath.json [--matcher Naive | SimpleGraph | DependenceGraph] [--start 0x0a0 | 016] [--end 0xb0 | 32] [--dumpSwitches]",name);
}

bool parseArgumens(IdiomMatcherStandalone &standalone, int argc, char *argv[]) {

    int c;
    bool success = true;
    while (1) {
        static struct option long_options[] =
                {
                        {"file",     required_argument, 0, 'f'},
                        {"patterns", required_argument, 0, 'p'},
                        {"matcher",  required_argument, 0, 'm'},
                        {"start",	 required_argument, 0, 's'},
                        {"end",		 required_argument, 0, 'e'},
                        {"dumpSwitches", no_argument, 0, 'd'},
                        {0,			 0,                 0,  0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "f:p:m:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;


        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'f':
                standalone.disassemblyFilePath = std::string(optarg);
                break;
            case 'p':
                standalone.patternFilePaths.push_back(std::string(optarg));
                break;
            case 'm':
                standalone.matcherQueue.push_back(std::string(optarg));
                break;
            case 's':
                standalone.startMatch = std::stoul(optarg,nullptr,0);
                break;
            case 'e':
                standalone.endMatch = std::stoul(optarg,nullptr,0);
                break;
            case 'd':
                standalone.shouldDumpSwitches = true;
                break;
            case '?':
                /* getopt_long already printed an error message. */
                success = false;
                break;
            default:
                abort();
        }
    }
    if (argc < 2)
        success = false;
    return success;
}