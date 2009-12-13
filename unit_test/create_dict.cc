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
当前采用MMSGE算法，还没有考虑规则4,生成chunk的时候自动剪枝去掉不足长的chunk满足规则1过滤，
然后在对与所有的带选chunk,同时按照规则2和规则3,用priority queue统一过滤。没有用两次过滤。
当前用的深度优先暴力搜索所有可能的分割方案，TODO 动态规划可以吗？速度会快吗？

长度(Length) chuck中各个词的长度之和 org.solol.mmseg.internal.Chunk.getLength() 
平均长度(Average Length) 长度(Length)/词数 org.solol.mmseg.internal.Chunk.getAverageLength() 
标准差的平方(Variance) 同数学中的定义 org.solol.mmseg.internal.Chunk.getVariance() 
自由语素度(Degree Of Morphemic Freedom) 各单字词词频的对数之和 org.solol.mmseg.internal.Chunk.getDegreeOfMorphemicFreedom() 

注意：表中的含义列可能有些模糊，最好参照MMSeg的源代码进行理解，代码所在的函数已经给出了。 

Chunk中的4个属性采用Lazy的方式来计算，即只有在需要该属性的值时才进行计算，而且只计算一次。

其次来理解一下规则(Rule)，它是MMSeg分词算法中的又一个关键的概念。实际上我们可以将规则理解为一个过滤器(Filter)，过滤掉不符合要求的chunk。MMSeg分词算法中涉及了4个规则：

规则1：取最大匹配的chunk (Rule 1: Maximum matching) 
规则2：取平均词长最大的chunk (Rule 2: Largest average word length) 
规则3：取词长标准差最小的chunk (Rule 3: Smallest variance of word lengths) 
规则4：取单字词自由语素度之和最大的chunk (Rule 4: Largest sum of degree of morphemic freedom of one-character words) 

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

//string infilename("cedict.txt");
string infilename("simple.log");
ofstream ofs("result.unicode.text");
utf16_filebuf<> out(ofs.rdbuf());

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

std::string u16string2string(const std::u16string& src)
{
  std::string des;
  u16string2string(src, des);
  return des;
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

  //ofstream ofs("dict.txt");
  ofstream ofs("dict2.txt");
  write_header(ofs.rdbuf());  //write utf16 little edian header
  utf16_filebuf<> obuf(ofs.rdbuf());

  for (; iter != end; ++iter) {
    if ((*iter)[0] == L'x' && (*iter)[1] == L':')
      continue;
    //string s;
    u16string s2;
    //s2 = (*iter).substr(0, ((*iter).find_first_of(L' ', 0) - (*iter).begin()));
    for (int i = 0; i < (*iter).size(); i++) {
      //int index = index_map((*iter)[i]);
      //if (index < 0 || index >= CJKSIZE + SYMBOLNUM)
      //  break;
      if ( (*iter)[i] < CJKBEGIN || (*iter)[i] >= CJKEND)
        break;
      s2.push_back((*iter)[i]);
    }
    //u16string2string(s2, s);
    //u16string2string(*iter, s);
    cout << u16string2string(s2) << endl;
  
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

void print_chunk(const std::u16string& sentence, const std::vector<int>& chunk_vec)
{
  using namespace std;
  int start = 0;
  for (int j = 0; j < chunk_vec.size(); j++) {
    //out << u16string2string(sentence.substr(start, chunk_vec[j])) << " ";
    for (int i = start; i < start + chunk_vec[j]; i++)
      out.sputc(sentence[i]);
    out.sputc(L' ');
    start += chunk_vec[j];
  }

  //最后是标点
  if (start < sentence.size() - 1) {
    //out << u16string2string(sentence.substr(start, sentence.size() - start)) << " ";
    for (int i = start; i < sentence.size() - 1; i++)
      out.sputc(sentence[i]);
    out.sputc(L' ');
  } 
  out.sputc(sentence[sentence.size() - 1]);
  out.sputc(L' ');
}

//s start, e end,
//TODO 有必要改为非递归？
//当前是深度优先全搜，有必要改成动态规划？但那要多计算不少chunk 分值
void do_split_word(const std::u16string& sentence, int s, int e,
                   WordHashMap hash_array[], 
                   std::vector<std::vector<int> >& chunk_vecs, 
                   std::vector<int>& chunk_vec) 
{
  //如果这个单字不构成任何词,包括它自身(新字)
  int index = index_map(sentence[s]);
  if (!is_index_in_map(index) || hash_array[index].empty()) {
    chunk_vec.push_back(1);
    if (s + 1 < e)
      do_split_word(sentence, s+1, e, hash_array, chunk_vecs, chunk_vec);
    else 
      chunk_vecs.push_back(chunk_vec);
    chunk_vec.pop_back();
    return;
  }
  
  std::u16string str;
  for (int i = s; i < e; i++) {
    str.push_back(sentence[i]);
    auto iter = hash_array[index].find(str);
    if (iter != hash_array[index].end() || str.size() == 1) {
      chunk_vec.push_back(i - s + 1);
      if (i + 1 < e)
        do_split_word(sentence, i+1, e, hash_array, chunk_vecs, chunk_vec);
      else {  //这里算是一个对句子的完整分词，与MMG原始算法不同我不记录标准1,因为不足长的不需要记录
        chunk_vecs.push_back(chunk_vec);
        chunk_vec.pop_back();
        return;
      }
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

  void set_data(int ind, double avl, double avv) {
    index = ind;
    avg_length = avl;
    avg_variance = avv;
  }

  bool operator < ( const ChunkInfo &other ) const {
      if ( (avg_length - other.avg_length) > 0.00000001 || (other.avg_length -avg_length) > 0.00000001)
        return avg_length < other.avg_length;
      else 
        return avg_variance > other.avg_variance;
  }

};

int find_best_chunk(const std::vector<std::vector<int> >& chunk_vecs)
{
  std::priority_queue<ChunkInfo> candidates;

  double min_avg_variance = 1;
  ChunkInfo cinfo;
  //TODO speed up
  for (int i = 0; i < chunk_vecs.size(); i++) {
    cinfo.set_data(i, get_avg_length(chunk_vecs[i]), 
                   get_variance(chunk_vecs[i]));
    
    candidates.push(cinfo);
  }
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

  //当前假设输入的sentence是末尾以标点之类结束的所以去掉它处理
  do_split_word(sentence, 0, sentence.size() - 1, hash_array, chunk_vecs, chunk_vec);
 
//#ifdef DEBUG
//  //检查各个chunk的划分情况
//  for (int i = 0; i < chunk_vecs.size(); i++) {
//    print_chunk(sentence, chunk_vecs[i]);
//  }
////#endif
  //显示分词结果
  int best_chunck_index = find_best_chunk(chunk_vecs);
  print_chunk(sentence, chunk_vecs[best_chunck_index]);
}

void word_segment()
{
  //ifstream ifs("normal_world.unicode.log");
  ifstream ifs(infilename.c_str());
  filebuf* pbuf = ifs.rdbuf();
  EncodingType encoding_type = read_header(pbuf);

  filebuf* pobuf = ofs.rdbuf();
  write_header(pobuf);   //mark utf16 little edidan coding

  sentence_iterator<> iter(pbuf);
  sentence_iterator<> end;

  for (;iter!=end; ++iter) {
    if (iter->empty())
      continue;
    //string s;
    //u16string2string(*iter, s);
    //cout << s <<  endl;
   if ((*iter).size() == 1) {
      out.sputc((*iter)[0]);
      out.sputc(L' ');
    }
    else if ((*iter).size() <= 30){
      split_word(*iter, hash_array);
    } 
    else {
      std::u16string t1 = ((*iter).substr(0,15));
      t1.push_back(0xfffe);
      split_word(t1, hash_array);
      std::u16string t2 = ((*iter).substr(15,(*iter).size() - 15));
      split_word(t2, hash_array);
    }
  }

  ifs.close();
  ofs.close();
}

TEST(test_dict, perf)
{
  //create_dict(infilename);
  

  create_dict_structure("dict3.txt", hash_array);
  cout << "finshed set up dict" << endl;
  //test_sentence_iter();
  //split_word((const char16_t*)L"研究生命起源", hash_array); //这里注意需要转换而且要改编译选项-fshort-wchar不然4byte一个字符
  //split_word((const char16_t*)L"起源", hash_array);
  //split_word((const char16_t*)L"街巷背阴的地方", hash_array);
  //split_word((const char16_t*)L"只有在半山腰县立高中的大院坝里", hash_array);
  //split_word((const char16_t*)L"隐隐约约", hash_array);
  //split_word((const char16_t*)L"隐隐约约听见远处传来一阵叮叮咣咣的声音", hash_array);
  //
  ////split_word((const char16_t*)L"他好象隐隐约约听见远处传来一阵“叮叮咣咣”的声音", hash_array);
  //split_word((const char16_t*)L"火水", hash_array);
  
  word_segment();
  

  //std::string str("unigram.txt");
  //create_dict(str);
}

int main(int argc, char *argv[])
{
  if (argc > 1)
    infilename = argv[1];

  //boost::progress_timer timer; 
  //testing::GTEST_FLAG(output) = "xml:";
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
