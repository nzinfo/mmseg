#include <stdio.h>
#include <iostream>
#include <glog/logging.h>

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);    // 初始化
    LOG(INFO) << "hello glog";
    printf("hello world.\n");
    return 0;
}

/* -- end of file --  */
