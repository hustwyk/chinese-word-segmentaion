/** 
 *  ==============================================================================
 * 
 *          \file   create_dict.cc
 *
 *        \author   chenghuige at gmail.com 
 *
 *          \date   2009-12-10 12:21:54.835564
 *  
 *   Description:   Create one dict for segmentaion from /build/bin/cedict.txt
 *
 *  ==============================================================================
 */

#include <iostream>
#include <fstream>
#include <gtest/gtest.h> 
#include <boost/progress.hpp>
//for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <assert.h>
#include "header_help.h"
#include "unicode_iterator.h"
#include "type.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include "memory.h"

using namespace std;
using namespace glseg;

string infilename("cedict.txt");

UTF8* unicode2utf8(UTF16 unicode, UTF8* utf8_array) {
    //memset(utf8,0,4);
    //only consider those ascii <128
    //and those >= 128 let it be 3 bytes do not consider other cases like 2 bytes!!
    if (unicode >= 128) {
        utf8_array[0] = 0xE0 | (unicode >> 12);
        utf8_array[1] = 0x80 | ((unicode >> 6)&0x3F);
        utf8_array[2] = 0x80 | (unicode & 0x3F);
        utf8_array[3] = '\0';
    } else {
        utf8_array[0] = (UTF8) unicode;
        utf8_array[1] = '\0';
    }

    return utf8_array;
}

void u16string2string(const std::u16string& src, std::string& des)
{
  glseg::UTF8 utf8_array[4];
  des.clear();
  for (int i = 0; i < src.size(); i++) {
    unicode2utf8(src[i], utf8_array);
    for(int j = 0; utf8_array[j] != '\0'; j++)
      des.push_back((char) utf8_array[j]);
  }
}

///这里尝试建立一个内存中的字典，并且序列话输出，以后用的时候
//直接序列化读入内存,格式是这样的一个数组，每一个对应一个汉字，
//数组的元素是哈希表，如字 兰，去数组直接索引得到数组位置，再
//去对应的哈希表找到如 兰花，兰州 等词。。。
//输入就是一个存在文本的词典
//? 是这样比较好 还是直接整个hash 所有的词? 这样用的空间可能多一点
//总之还是需要进一步了解GCC hash table的内部构造 TODO
WordHashMap hash_array[CJKSIZE + SYMBOLNUM];

void create_dict_structure(const char* infilename, WordHashMap hash_array[])
{
  ifstream ifs(infilename);
  filebuf* pbuf = ifs.rdbuf();
  EncodingType encoding_type = read_header(pbuf);
  assert(encoding_type == Utf16LittleEndian);

  
  unicode_line_iterator<> iter(pbuf);
  unicode_line_iterator<> end;

    WordInfo winfo;

  //按照最初的想法我只是需要一个hash能够快速确定key在不在而不需要map, TODO
  size_t index;
  for (; iter != end; ++iter) {
    index = index_map((*iter)[0]);
    if (index >= 0 && index < (CJKSIZE + SYMBOLNUM)) //TODO may be fixed array?
      hash_array[index][*iter] = winfo;  
  }

  //test
#ifdef DEBUG2
  for (auto iter = hash_array[index_map(L'兰')].begin(); 
        iter != hash_array[index_map(L'兰')].end();
        ++iter) {
    string s;
    u16string2string((*iter).first, s);
    cout << s << endl;
  }
#endif

//  //------------序列化到文本
//  std::ofstream ofs("dict_struct.txt", std::ios::binary);//把对象写到file.txt文件中
//  boost::archive::binary_oarchive oa(ofs);   //文本的输出归档类，使用一个ostream来构造
//
//  oa << hash_array;
//
//  ofs.close();
//
//  //---test 从序列话后的文本读出
//#ifdef DEBUG
//  cout << "reading dict struct " << endl;
//  WordHashMap hash_array2[CJKSIZE + SYMBOLNUM];
//  std::ifstream ifs("dict_struct.txt", std::ios::binary);//把对象写到file.txt文件中
//  boost::archive::binary_iarchive ia(ifs);   //文本的输出归档类，使用一个ostream来构造
//  ia >> hash_array2;
//  for (auto iter = hash_array2[index_map(L'兰')].begin(); 
//        iter != hash_array2[index_map(L'兰')].end();
//        ++iter) {
//    string s;
//    u16string2string((*iter).first, s);
//    cout << s << endl;
//  }
//  ifs.close();
//#endif
//

  ////按照最初的想法我只是需要一个hash能够快速确定key在不在而不需要map, TODO
  //UTF8WordHashMap hash_array[CJKSIZE + SYMBOLNUM];
  //WordInfo winfo;

  //UTF8 utf8string[4];

  //size_t index;
  //for (int i = 0; iter != end; ++iter, i++) {
  //  //assert( (*iter)[0] >= CJKBEGIN && (*iter)[0] < CJKEND);
  //  //EXPECT_GE(index_map((*iter)[0]), 0) << " " << (*iter)[0] << " " <<  unicode2utf8((*iter)[0], utf8string) << i;
  //  //EXPECT_LT(index_map((*iter)[0]), CJKEND + SYMBOLNUM) << " " << (*iter)[0] << " " <<  unicode2utf8((*iter)[0], utf8string) << i;
  //  
  //  string s;
  //  u16string2string(*iter, s);

  //  index = index_map((*iter)[0]);
  //  if (index > 0 && index < CJKBEGIN + SYMBOLNUM)
  //    hash_array[index][s] = winfo;  
  //}

  ////test
  //for (auto iter = hash_array[index_map(L'你')].begin(); 
  //      iter != hash_array[index_map(L'你')].end();
  //      ++iter) {
  //  cout << (*iter).first << endl;
  //}

}

///this will create a file "dict.txt" from "cedict.txt"
void create_dict(string infilename)
{
  ifstream ifs(infilename.c_str());
  filebuf* pbuf = ifs.rdbuf();
  EncodingType encoding_type = read_header(pbuf);

  assert(encoding_type == Utf16LittleEndian);

  unicode_line_iterator<> iter(pbuf);
  unicode_line_iterator<> end;

  ofstream ofs("dict.txt");
  write_header(ofs.rdbuf());  //write utf16 little edian header
  utf16_filebuf<> obuf(ofs.rdbuf());

  for (; iter != end; ++iter) {
    string s;
    u16string s2;
    s2 = (*iter).substr(0, (*iter).find_first_of(L' ', 0));
    u16string2string(s2, s);
    //u16string2string(*iter, s);
    //cout << s << endl;
  
    for (int i = 0; i < s2.size(); i++)
      obuf.sputc(s2[i]);
    obuf.sputc(L'\n');

  }
  //utf16_istreambuf_iterator<> first(pbuf);
  //utf16_istreambuf_iterator<> end;

  //UTF8 utf8_array[4];
  //string s;
  //for (; first != end; ++first) {
  //  //unicode2utf8(*first, utf8_array);
  //  //string s((const char*)utf8_array);
  //  //cout << utf8_array;
  //  //cout << s;
  //}

  ifs.close();
  ofs.close();
}

void test_sentence_iter()
{
  ifstream ifs("normal_world.unicode.log");
  filebuf* pbuf = ifs.rdbuf();
  EncodingType encoding_type = read_header(pbuf);

  sentence_iterator<> iter(pbuf);
  sentence_iterator<> end;

  for (;iter!=end; ++iter) {
    if (iter->empty())
      continue;
    string s;
    u16string2string(*iter, s);
    cout << s <<  endl;
  }

  ifs.close();
}


//s start, e end,
void do_split_word(const std::u16string& sentence, int s, int e,
                   WordHashMap hash_array[], 
                   std::vector<std::vector<int> >& chunk_vecs, 
                   std::vector<int>& chunk_vec) 
{
  if (s == e) {
    chunk_vecs.push_back(chunk_vec);
    return;
  }
  
  //如果这个单字不构成任何词,包括它自身(新字:))
  if (hash_array[index_map(sentence[s])].empty()) {
    cout << "new word??" << endl;
    chunk_vec.push_back(1);
    do_split_word(sentence, s+1, e, hash_array, chunk_vecs, chunk_vec);
    chunk_vec.pop_back();
    return;
  }

  std::u16string str;
  for (int i = s; i < e; i++) {
    str.push_back(sentence[i]);
    auto iter = hash_array[index_map(sentence[s])].find(str);
    if (iter != hash_array[index_map(sentence[s])].end()) {
      chunk_vec.push_back(i - s + 1);
      do_split_word(sentence, i+1, e, hash_array, chunk_vecs, chunk_vec);
      chunk_vec.pop_back();
    }
  }
}

inline double get_avg_length(const std::vector<int>& chunk_vec) 
{
  return std::accumulate(chunk_vec.begin(), chunk_vec.end(), 0) /(double)chunk_vec.size();
}

inline double get_variance(const std::vector<int>& chunk_vec)
{
  int avg = get_avg_length(chunk_vec);
  double variance = 0;
  int l;
  for (int i = 0; i < chunk_vec.size(); i++) {
    l = chunk_vec[i] - avg;
    variance += (double)l*l;
  }
  return variance/(double)chunk_vec.size();
}

struct ChunkInfo {
  int index;
  double avg_length;
  double avg_variance;

  bool operator < ( const ChunkInfo &other ) const {
      //if not equal
      if ( (avg_length - other.avg_length) > 0.00000001 || (other.avg_length -avg_length) > 0.00000001)
        return avg_length < other.avg_length;
      else 
        return avg_variance > other.avg_variance;
  }

};

int find_best_chunk(const std::vector<std::vector<int> >& chunk_vecs)
{
  //typedef std::deque<int> HuffDQU;   
  //typedef std::priority_queue<int, HuffDQU, HuffNodeIndexGreater>  HuffPRQUE; 

   std::priority_queue<ChunkInfo> candidates;

  double min_avg_variance = 1;
  ChunkInfo cinfo;
  //TODO speed up
  for (int i = 0; i < chunk_vecs.size(); i++) {
    cinfo.index = i;
    cinfo.avg_length = get_avg_length(chunk_vecs[i]);
    cinfo.avg_variance = get_variance(chunk_vecs[i]);
    candidates.push(cinfo);
    //cout << cinfo.avg_length << endl;
  }
  //cout << candidates.top().avg_length << endl;
  return candidates.top().index;
}

void split_word(const std::u16string& sentence, WordHashMap hash_array[]) 
{
  //string s;
  //u16string2string(sentence, s);
  //cout << s << endl;
  
  //TODO learn more about vec
  //chunk vec具体存储各种可能划分所得到的各个词的长度
  std::vector<std::vector<int> > chunk_vecs;
  std::vector<int> chunk_vec;
  do_split_word(sentence, 0, sentence.size(), hash_array, chunk_vecs, chunk_vec);
 
  std::string str;

////#ifdef DEBUG
//  //检查各个chunk的划分情况
//   for (int i = 0; i < chunk_vecs.size(); i++) {
//    int s = 0;
//    for (int j = 0; j < chunk_vecs[i].size(); j++) {
//      //cout << "*" << chunk_vecs[i][j] << " " << s << " " << chunk_vecs[i][j] <<" ";
//
//      u16string2string(sentence.substr(s, chunk_vecs[i][j]), str);
//      cout << str << "$";
//      s += chunk_vecs[i][j];
//    }
//    cout << endl;
//  }
////#endif
//
  //显示分词结果
  int best_chunck_index = find_best_chunk(chunk_vecs);
    int start = 0;
    for (int j = 0; j < chunk_vecs[best_chunck_index].size(); j++) {
      //cout << "*" << chunk_vecs[i][j] << " " << s << " " << chunk_vecs[i][j] <<" ";
      u16string2string(sentence.substr(start, chunk_vecs[best_chunck_index][j]), str);
      cout << str << "$";
      start += chunk_vecs[best_chunck_index][j];
    }
    cout << endl;

}

void word_segment()
{
  ifstream ifs("normal_world.unicode.log");
  filebuf* pbuf = ifs.rdbuf();
  EncodingType encoding_type = read_header(pbuf);

  sentence_iterator<> iter(pbuf);
  sentence_iterator<> end;

  for (;iter!=end; ++iter) {
    if (iter->empty())
      continue;
    string s;
    u16string2string(*iter, s);
    cout << s <<  endl;

    split_word(*iter, hash_array);
  }

  ifs.close();

}

TEST(test_dict, perf)
{
  //create_dict(infilename);
  create_dict_structure("dict.txt", hash_array);
  //test_sentence_iter();
  //split_word((const char16_t*)L"研究生命起源", hash_array); //这里注意需要转换而且要改编译选项-fshort-wchar不然4byte一个字符
  //split_word((const char16_t*)L"起源", hash_array);
  //split_word((const char16_t*)L"街巷背阴的地方", hash_array);
  word_segment();
}

int main(int argc, char *argv[])
{
  //boost::progress_timer timer; 
  //testing::GTEST_FLAG(output) = "xml:";
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
