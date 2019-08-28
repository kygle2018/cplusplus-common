#include "clib_json.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <fstream>
#include <stdlib.h>

using namespace std;

struct  PictureInfo {
    string name;
    float rotation;
} gPictureInfo;

#define JSON_GET_MUST_ELEM(root, elem_str, value, elemType)  do{\
        isExist = root[elem_str].isNull();\
        if(isExist == true){\
            printf("not exist!!!\n");\
        }\
        value = root[elem_str].as##elemType();\
    }while(0)\
         
void  PictureInit()
{
    clib_json::Reader reader;
    ifstream file("./test.json");
    assert(file.is_open());
    clib_json::Value root;
    if (!reader.parse(file, root, false)) {
        printf("Parse error");
        exit(1);
    }
    bool isExist = false;
    clib_json::Value header = root["req_header"];
    unsigned int uin, ver = 0;
    //JSON_GET_MUST_ELEM(header, "uin", uin, UInt );
    string know1 = header["cmd"].asString();
    JSON_GET_MUST_ELEM(header, "uin", uin, UInt);
    JSON_GET_MUST_ELEM(header, "proto_ver", ver, UInt);
    printf("uin:%u\n  know1:%s\n    ver:%u", uin, know1.c_str(), ver);
    return;
    gPictureInfo.name = root["name" ].asString();
    gPictureInfo.rotation = root["rotation" ].asDouble();
}


int main()
{
    //encode example.
    clib_json::Value  root;
    clib_json::Value  var;

    var["token"] = "haha";
    var["game_id"] = 1;

    clib_json::Value  pieces;//store all pieces
    for (int i = 0; i < 3; i++) {
        clib_json::Value  piece_ex;//here it store just one piece
        piece_ex["x"] = 100 + i;
        piece_ex["y"] = 200 + i;
        pieces.append(piece_ex);//ok,yes we just have apply One piece ,then push back to the array
    }

    var["piece_array"] = pieces;//yes,store pieces in var [Value]
    root.append(var);

    clib_json::FastWriter  writer;
    string strResult = writer.write(root);//generate json string:),here all is done

    printf("%s\n", strResult.c_str());


    //decode example.
    printf("decode sample:\n");
    PictureInit();
    printf("%s %f\n", gPictureInfo.name.c_str(), gPictureInfo.rotation);

    return 0;
}

