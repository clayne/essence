#include <vector>
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

    const std::string &DeclarationManager::getRandomDummyVariableName() {
        return TmpStringCandidates[rand() % TmpStringCandidates.size()];
    }

    std::string DeclarationManager::getUniqueTmpCPPVariableNameFor() {
        return getUniqueTmpCPPVariableNameFor(getRandomDummyVariableName());
    }

    bool DeclarationManager::isNameDefined(std::string name) const {
        bool defined_as_tmp_variable = std::find(std::begin(this->definedNamesForFunctionBeingGenerated),
                                                 std::end(this->definedNamesForFunctionBeingGenerated), name) !=
                                       std::end(this->definedNamesForFunctionBeingGenerated);


        bool reserved_anyway = std::find_if(std::begin(this->otherDisallowedNames), std::end(this->otherDisallowedNames),
                                            [&name](const std::string &value) { return value == name; }) !=
                               std::end(this->otherDisallowedNames);

        bool defined_as_global = std::find_if(std::begin(this->globals), std::end(this->globals),
                                              [&name](const GlobalVariable &g) { return g.getName() == name; }) !=
                                 std::end(this->globals);

        return defined_as_tmp_variable || defined_as_global || reserved_anyway;
    }

    std::string DeclarationManager::getUniqueTmpCPPVariableNameFor(const std::string &input) {
        // This is explicitly not random
        auto candidate = input;
        int counter = 0;
        while (true) {
            candidate.append(getRandomDummyVariableName());
            counter++;

            if (!this->isNameDefined(candidate)) {
                definedNamesForFunctionBeingGenerated.push_back(candidate);
                return candidate;
            }
        }
    }


    std::string DeclarationManager::getUniqueLoopIteratorName() {
        auto it_name = getUniqueTmpCPPVariableNameFor();
        iteratorNames.push_back(it_name);
        return it_name;
    }


    std::string DeclarationManager::joinStrings(const std::vector<std::string>& strings, StringJoiningFormat format) const {
        std::stringstream output;
        std::string delimiter;
        if (format == GENERATE_FORMAT_CPP_ADDRESSING)
            delimiter = CPP_ADDRESSING_DELIMITER;
        if (format == GENERATE_FORMAT_CPP_VARIABLE)
            delimiter = LVALUE_DELIMITER;

        bool hasSkippedRoot = false;

        for (auto &s : strings) {
            if (format != GENERATE_FORMAT_CPP_VARIABLE && s == POINTER_DENOTATION)
                continue; // don't print the pointer markings in lvalue as these are abstracted away
            if (format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING ||
                format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING_WITHOUT_ROOT) {
                if (format == GENERATE_FORMAT_JSON_ARRAY_ADDRESSING_WITHOUT_ROOT && !hasSkippedRoot) {
                    hasSkippedRoot = true;
                    continue;
                }
                if (std::find(iteratorNames.begin(), iteratorNames.end(), s) != iteratorNames.end())
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
            freeVectorVariableName = getUniqueTmpCPPVariableNameFor("vectorKeepingVariableNamesToBeFreed");
            freeVectorNameHasBeenSet = true;
        }
        return freeVectorVariableName;
    }


    std::string DeclarationManager::registerVariableToBeFreed(const std::string &variable_name) {
        std::stringstream output;
        output << getFreeVectorName() << ".push_back((void*)" << variable_name << ");";
        return output.str();
    }

    void DeclarationManager::addDeclaration(Type *f) {
        this->userDefinedTypes.push_back(f);
    }

    void DeclarationManager::addDeclaration(const GlobalVariable &gv) {
        this->globals.push_back(gv);
    }

    void DeclarationManager::addDeclaration(const std::string &reserve_name) {
        this->otherDisallowedNames.push_back(reserve_name);
    }

    void DeclarationManager::clearGeneratedNames() {
        definedNamesForFunctionBeingGenerated.clear();
        iteratorNames.clear();
    }
}