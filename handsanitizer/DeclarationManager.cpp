#include <vector>
#include "DeclarationManager.h"
#include "DeclarationManager.h"

namespace handsanitizer {
    const std::string CPP_ADDRESSING_DELIMITER = ".";
    const std::string LVALUE_DELIMITER = "_";
    const std::string POINTER_DENOTATION = "__p";

    /*
     * This generates a unique name that is also distinct compared to its argument!
     * This is a required to make sure that there is no accidental way to generate correct code by incorrectly calling this
     * As names generated by this should not conflict with argument  names, even though argument names can be passed in.
     * If this function were not to guarantee uniqueness w.r.t it's arguments, then it would depend on the order which this function is called
     * and implicit orderings are bad
    */
    std::string DeclarationManager::getUniqueTmpCPPVariableNameFor(std::vector<std::string> prefixes) {
        auto candidate = joinStrings(prefixes, GENERATE_FORMAT_CPP_VARIABLE);

        // This is explicitly not random
        int counter = 0;
        while (true) {
            candidate = candidate + getRandomDummyVariableName();
            counter++;

            if (!this->isNameDefined(candidate)) {
                definedNamesForFunctionBeingGenerated.push_back(candidate);
                return candidate;
            }
        }
    }

    std::vector<std::string> TmpStringCandidates{
            "foo",
            "bar",
            "baz",
            "alice",
            "bob",
            "carlos",
            "dan",
            "erin",
            "eve",
            "judy",
            "levi",
            "micheal",
            "panda", // hi my name is
            "slim",
            "shady",
            "vera",
            "victor",
            "ted",
    };

    std::string DeclarationManager::getRandomDummyVariableName() {
        return TmpStringCandidates[rand() % TmpStringCandidates.size()];
    }

    std::string DeclarationManager::getUniqueTmpCPPVariableNameFor() {
        std::vector<std::string> vec;
        vec.push_back(getRandomDummyVariableName());
        return getUniqueTmpCPPVariableNameFor(vec);
    }

    bool DeclarationManager::isNameDefined(std::string name) {
        bool defined_as_tmp_variable = std::find(std::begin(this->definedNamesForFunctionBeingGenerated),
                                                 std::end(this->definedNamesForFunctionBeingGenerated), name) !=
                                       std::end(this->definedNamesForFunctionBeingGenerated);


        bool reserved_anyway = std::find_if(std::begin(this->other_disallowed_names), std::end(this->other_disallowed_names),
                                                [&name](std::string value) { return value == name; }) !=
                                   std::end(this->other_disallowed_names);

        bool defined_as_global = std::find_if(std::begin(this->globals), std::end(this->globals),
                                              [&name](GlobalVariable &g) { return g.getName() == name; }) !=
                                 std::end(this->globals);

        return defined_as_tmp_variable || defined_as_global || reserved_anyway;
    }

    std::string DeclarationManager::getUniqueTmpCPPVariableNameFor(std::string prefix) {
        std::vector<std::string> vec;
        vec.push_back(prefix);
        return getUniqueTmpCPPVariableNameFor(vec);
    }


    std::string DeclarationManager::getUniqueLoopIteratorName() {
        auto it_name = getUniqueTmpCPPVariableNameFor();
        iterator_names.push_back(it_name);
        return it_name;
    }


    std::string DeclarationManager::joinStrings(std::vector<std::string> strings, StringJoiningFormat format) {
        std::stringstream output;
        std::string delimiter = "";
        if (format == GENERATE_FORMAT_CPP_ADDRESSING)
            delimiter = CPP_ADDRESSING_DELIMITER;
        if (format == GENERATE_FORMAT_CPP_VARIABLE)
            delimiter = LVALUE_DELIMITER;

        bool hasSkippedRoot = false;

        for (std::string s : strings) {
            if (format != GENERATE_FORMAT_CPP_VARIABLE && s == POINTER_DENOTATION)
                continue; // don't print the pointer markings in lvalue as these are abstracted away
            if (format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING ||
                format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING_WITHOUT_ROOT) {
                if (format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING_WITHOUT_ROOT && !hasSkippedRoot) {
                    hasSkippedRoot = true;
                    continue;
                }
                if (std::find(iterator_names.begin(), iterator_names.end(), s) != iterator_names.end())
                    output << "[" << s << "]";
                else
                    output << "[\"" << s << "\"]";
            } else {
                output << s << delimiter;
            }
        }

        auto retstring = output.str();
        if (retstring.length() > 0 && format != GENERATE_FORMAT_JSON_ARRAY_ADDRESSING &&
            format != GENERATE_FORMAT_JSON_ARRAY_ADDRESSING_WITHOUT_ROOT)
            retstring.pop_back(); //remove trailing delimiter
        return retstring;
    }


    std::string DeclarationManager::getFreeVectorName() {
        if (!freeVectorNameHasBeenSet) {
            freeVectorVariableName = this->getUniqueTmpCPPVariableNameFor("vectorKeepingVariableNamesToBeFreed");
            freeVectorNameHasBeenSet = true;
        }
        return freeVectorVariableName;
    }


    std::string DeclarationManager::registerVariableToBeFreed(std::string variable_name) {
        std::stringstream output;
        output << getFreeVectorName() << ".push_back((void*)" << variable_name << ");";
        return output.str();
    }

    void DeclarationManager::addDeclaration(Type *f) {
        this->user_defined_types.push_back(f);
    }

    void DeclarationManager::addDeclaration(const GlobalVariable &gv) {
        this->globals.push_back(gv);
    }

    void DeclarationManager::addDeclaration(const std::string &reserve_name) {
        this->other_disallowed_names.push_back(reserve_name);
    }

    void DeclarationManager::clearGeneratedNames() {
        definedNamesForFunctionBeingGenerated.clear();
        iterator_names.clear();
    }
}