#include <opencv2/opencv.hpp>
#include <string>
#ifndef NET_DECODE_H
#define NET_DECODE_H
std::string base64Decode(const char* Data, int DataByte);
std::string base64Encode(const unsigned char* Data, int DataByte);
std::string Mat2Base64(const cv::Mat &img, std::string imgType);
cv::Mat Base2Mat(std::string &base64_data);
#endif
