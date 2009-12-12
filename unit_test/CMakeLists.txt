#set (test_iter_srcs test_iter.cc ../utils/src/unicode_helper.cc ../utils/src/ConvertUTF.cc )

set (test_iter_srcs test_iter.cc  ../utils/src/ConvertUTF.cc ../utils/src/header_help.cc)
add_executable (test_iter ${test_iter_srcs})


set (create_dict_srcs create_dict.cc ../utils/src/ConvertUTF.cc ../utils/src/header_help.cc)
add_executable (create_dict ${create_dict_srcs})

#target_link_libraries(utest libboost_python.so libpython2.6.so)


#set (test_tokenizer_srcs test_tokenizer.cc)
#add_executable (test_tokenizer ${test_tokenizer_srcs})
#target_link_libraries(test_tokenizer libboost_filesystem.so)

#set (test_huffword_srcs test_huffword.cc)
#add_executable(test_huffword ${test_huffword_srcs})
#target_link_libraries(test_huffword libboost_filesystem.so libboost_serialization.so)




