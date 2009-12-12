/** 
 *  ==============================================================================
 * 
 *          \file   test_iter.cc
 *
 *        \author   chenghuige at gmail.com 
 *
 *          \date   2009-12-09 16:41:16.778308
 *  
 *   Description:   Test the iterator class 
 *                  Input: a file or a range with begin end mark
 *                         The file must be encoded in utf16 or utf8, if utf8 will
 *                         convert each character to utf16
 *                  Output: the unsigned short as one utf16 character for *iter
 *
 *  ==============================================================================
 */

#include <iostream>
#include <fstream>
#include <gtest/gtest.h> 
#include <boost/progress.hpp>

#include "header_help.h"
#include "unicode_iterator.h"
#include "ConvertUTF.h"
#include "type.h"
using namespace std;
using namespace glseg;

string infilename("normal_world.unicode.log");

void unicode2utf8(UTF16 unicode, UTF8* utf8_array) {
    //memset(utf8,0,4);
    if (unicode >= 128) {
        utf8_array[0] = 0xE0 | (unicode >> 12);
        utf8_array[1] = 0x80 | ((unicode >> 6)&0x3F);
        utf8_array[2] = 0x80 | (unicode & 0x3F);
        utf8_array[3] = '\0';
    } else {
        utf8_array[0] = (UTF8) unicode;
        utf8_array[1] = '\0';
    }
}

void test_iter() {

    UTF16 utf16 = L',';
    cout << utf16 << endl;

    utf16 = L'。';
    cout << utf16 << endl;

    utf16 = L'你';
    cout << utf16 << endl;

    string s;

    //test untf converter
    //TODO understand and modify the api to only handle one utf16 character to utf8
    //UTF16 unicode[]= {L'你', L'w',L'u'};
    //UTF8 utf8[10];
    //utf8[3] = '\0';
    ConversionResult result = sourceIllegal;

    UTF16 utf16_buf[8] = {0};

    ////utf16_buf[0] = 0xd834;

    utf16_buf[0] = L'你';

    utf16_buf[1] = L'a';


    //utf16_buf[1] = 0xdf00;

    //utf16_buf[2] = 0xd834;

    //utf16_buf[3] = 0xdf01;

    //utf16_buf[4] = 0xd834;

    //utf16_buf[5] = 0xdf02;

    //utf16_buf[6] = 0;

    //utf16_buf[7] = 0;

    UTF16 *utf16Start = utf16_buf;

    UTF8 utf8_buf[16] = {0};

    UTF8* utf8Start = utf8_buf;

    
    //notice can not use &utf16_buf!
    result = ConvertUTF16toUTF8((const UTF16 **) & utf16Start, utf16_buf + 2, &utf8Start, utf8_buf + 16);
    cout << utf8_buf << endl;
    cout << "haha" << endl;

    switch (result) {

        default: fprintf(stderr, "Test02B fatal error: result %d for input %08x\n", result, utf16_buf[0]);
            exit(1);

        case conversionOK: break;

        case sourceExhausted: printf("sourceExhausted\t");
            exit(0);

        case targetExhausted: printf("targetExhausted\t");
            exit(0);

        case sourceIllegal: printf("sourceIllegal\t");
            exit(0);

    }


    ifstream istr(infilename.c_str());
    filebuf* pbuf = istr.rdbuf();

    //unsigned char ch = pbuf->sbumpc();
    //unsigned char ch1;
    //ch1 = pbuf->sbumpc();

    //if (ch == encoding_types[Utf16LittleEndian][0] && ch1 == encoding_types[Utf16LittleEndian][1])
    //    cout << "The encoding of this file is  utf16 little endian" << endl;
    //if (ch == encoding_types[Utf16BigEndian][0] && ch1 == encoding_types[Utf16BigEndian][1])
    //    cout << "The encoding of this file is  utf16 big endian" << endl;
    EncodingType encoding_type = read_header(pbuf);
    assert(encoding_type == Utf16LittleEndian);



    utf16_istreambuf_iterator<> first(pbuf);
    utf16_istreambuf_iterator<> end;

    UTF8 utf8_array[4];
    for (; first != end; ++first) {
        unicode2utf8(*first, utf8_array);
        cout << utf8_array;
    }
    //for (int num = 0;pbuf->sgetc()!=EOF; num++)
    //{
    //   ch = pbuf->sbumpc();
    //   ch1 = pbuf->sbumpc();
    //
    //   UTF16 unicode = ch1;
    //   unicode <<= 8;
    //   unicode |= ch;
    //
    //   UTF8 utf8_array[4];


    //unicode2utf8(unicode, utf8_array);

    //   cout << utf8_array;

    //   if (num > 20)
    //     break;
    //}

    istr.close();


}

TEST(test_convert, func) {
    test_iter();
}

int main(int argc, char *argv[]) {
    if (argc > 1)
        infilename = argv[1];

    //boost::progress_timer timer;

    //testing::GTEST_FLAG(output) = "xml:";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
