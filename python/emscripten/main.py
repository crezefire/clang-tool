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

def to_js_name(decl):
    decl.lower()

def get_param_list(params):
    str_list = ""
    i = 0
    for param in params:
        str_list += param["name"]

        if (i < len(params) - 1):
            str_list += ", "

        i += 1

    return str_list

def get_return_params(return_type):
    if return_type["type"] != "Void":
        return "\"number\""
    else:
        return "null"

def get_interop_params(params):
    str_list = ""
    i = 0
    for param in params:
        type =  param["type"]

        if type == "String":
            str_list += "\"string\""
        else:
            str_list += "\"number\""

        if (i < len(params) - 1):
            str_list += ", "

        i += 1

    return str_list

def get_llvm_ir_type(param):
    keyword = param["keyword"]

    if "float" in keyword:
        return ["float", "4"]
    elif "double" in keyword:
        return ["double", "8"]
    elif "int" in keyword:
        return ["int" , "4"]
    else:
        return ["", ""] #TODO(vim): Handle this

def get_marshal_params(params):
    marshal_decs = ""
    marshal_params = ""

    for param in params:
        type = param["type"]
        name = param["name"]

        if type == "String":
            marshal_decs += "\t\tvar " + name + "Marshal = emscriptenMemory.marshalString(" + name + ");"
            marshal_decs += " marshalPointers.push(" + name + "Marshal);\n"
            marshal_params += ", " + name + "Marshal"
        elif type == "Array" or type == "Pointer":
            ir_type, size = get_llvm_ir_type(param)
            marshal_decs += "\t\tvar " + name + "Marshal = emscriptenMemory.marshalArrayOfType(" + name + ", " + ir_type + ", " + size + ");\n"
            marshal_params += ", " + name + "Marshal"
        else:
            marshal_params += ", " + name

    return [marshal_decs, marshal_params]

def main(args):
    file = open(args["file"], "r")

#TODO(vim): file open error

    contents = file.read()

    data = json.loads(contents)

    for module in data["modules"]:
        module_name = module["name"]
        module_file = module["file-name"]
        module_ns = module["namespace"]

        out_file = open(args["out"] + module_name + ".js", "w")

        out_file.write("var emscriptenMemory = require(\"./emscripten_memory\");\n")
        out_file.write(next_block())

        api_pointer = "apiPointer"
        c_wrap = "cwrap"
        runtime = "runtime"

        out_file.write("function " + module_name + "(" + api_pointer + ", " + c_wrap + ", " + runtime + ") {")
        out_file.write(next_line())

        api_pointer = "_" + api_pointer
        out_file.write("\tvar " + api_pointer + " = apiPointer;\n")
        out_file.write(next_line())

        interop = "Interop"

        for method in module["methods"]:
            method_name = method["name"]
            out_file.write("\tvar _" + method_name + interop + " = null;\n")

        out_file.write(next_block())

        for method in module["methods"]:
            method_name = method["name"]
            method_return = method["return-type"]
            method_params = method["params"]

            param_list = get_param_list(method_params)
            return_type = get_return_params(method_return)
            interop_params_list = get_interop_params(method_params)
            marshal_decls, marshal_params_list = get_marshal_params(method_params)

            prepend = "\"number\""

            if (len(interop_params_list) > 0):
                prepend += ", "

            interop_params_list = prepend + interop_params_list

            out_file.write("\tthis." + method_name + " = function(" + param_list + ") {\n")
            out_file.write("\t\t_" + method_name + interop + " = _" + method_name + interop + " || " + c_wrap + "(\"" + method_name + "\", " + return_type + ", [" + interop_params_list +"]);\n\n")

            if len(marshal_decls) > 0:
                out_file.write("\t\tvar marshalPointers = [];\n\n")
                out_file.write(marshal_decls + "\n")

            out_file.write("\t\tvar retValue = _" + method_name + interop + "(_apiPointer" + marshal_params_list + ");\n\n")

            if len(marshal_decls) > 0:
                out_file.write("\t\temscriptenMemory.freeMemory(marshalPointers);\n")

            out_file.write("\t\treturn retValue;\n")
            out_file.write("\t};\n\n")

        out_file.write("}\n\nmodule.exports = " + module_name + ";")

if __name__ == "__main__":
    input_args = {}
    input_args["file"] = "../../zzz.json"
    input_args["out"] = "../../"
    main(input_args)