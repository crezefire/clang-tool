import json

def next_block():
    return "\n\n"

def next_line():
    return "\n"

def resolve_type(type):
    ret_string = type["keyword"]
    ret_type = type["type"]

    if (ret_type == "Pointer" or ret_type == "String"):
        ret_string += "*"
    elif (ret_type == "Array"):
        ret_string = ret_string[0:-3]

    return ret_string

def resolve_return_type(type):
    return resolve_type(type)

def resolve_param_type(type):
    type_decl = resolve_type(type)
    type_category = type["type"]
    type_name = type["name"]

    if (type_category == "FunctionPointer"):
        pos = type_decl.find("*")
        return type_decl[0:pos] + type_name + type_decl[pos:]

    return type_decl + " " + type_name

def main(args):
    file = open(args["file"], "r")

#TODO(vim): file open error

    contents = file.read()

    data = json.loads(contents)

    for module in data["modules"]:
        module_name = module["name"]
        module_file = module["file-name"]
        module_ns = module["namespace"]
        api_include_file = "EegeoMapApi.h"
        must_include_file = "EegeoApiDefs.h"

        out_file = open(args["out"] + module_name + "_gen.cpp", "w")
        out_file.write("#include \"" + module_file + "\"\n")
        out_file.write("#include \"" + must_include_file + "\"\n") #TODO(vim): Configurable
        out_file.write("#include \"" + api_include_file + "\"") #TODO(vim): Configurable
        out_file.write(next_block())

        out_file.write("extern \"C\"")
        out_file.write(next_line())
        out_file.write("{")
        out_file.write(next_line())

        for method in module["methods"]:
            method_name = method["name"]
            method_return = method["return-type"]
            method_params = method["params"]
            method_attribs = " EMSCRIPTEN_KEEPALIVE "

            out_file.write("\t")
            out_file.write(resolve_return_type(method_return) + method_attribs + method_name + "(")

            i = 0
#TODO(vim): Configurable
            out_file.write("Eegeo::Api::EegeoMapApi* pApi")

            if (len(method_params) > 0):
                out_file.write(", ")

            param_name_list = ""

            for param in method_params:
                out_file.write(resolve_param_type(param))
                param_name_list += param["name"]

                if (i != len(method_params) - 1):
                    out_file.write(", ")
                    param_name_list += ", "


                i += 1

            out_file.write(")\n\t{\n")

            local_api = "api"

            out_file.write("\t\t")
            out_file.write(module_ns + module_name + "* " + local_api + " = reinterpret_cast<" + module_ns + module_name + "*>(pApi->GetApi(\"" + module_name + "\"));\n\n")

            out_file.write("\t\t")
            out_file.write(local_api + "->" + method_name + "(" + param_name_list + ");\n")

            out_file.write("\t}\n\n")

        out_file.write("}\n")


if __name__ == "__main__":
    input_args = {}
    input_args["file"] = "../../zzz.json"
    input_args["out"] = "../../"
    main(input_args)