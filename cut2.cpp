#include "../opencv-3.4.0/include/opencv2/opencv.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace cv;

#define MAX_LEN 100

int HistB_prev[256]={0,};
int HistG_prev[256]={0,};
int HistR_prev[256]={0,};

int HistB_cur[256]={0,};
int HistG_cur[256]={0,};
int HistR_cur[256]={0,};


int HistB[256] = {0,};
int HistG[256] = {0,};
int HistR[256] = {0,};

int ret_R = 0;
int ret_G = 0;
int ret_B = 0;

//char* r_eal;
char buf1[256] = {0,};
char buf2[256] = {0,};
char buf3[256] = {0,};

void ret_time(double time, char *r_eal ){

    int hour, min;
    int time1 = time;
    time = time - time1;
    double sec;
    char *str ; 

    min = time1/60;
    hour = min/60;
    sec = time1%60 + time;
    min = min%60;

    if(hour < 10)
        sprintf(buf1, "0%d:", hour );
    else
        sprintf(buf1, "%d:", hour);

    if(min < 10)
        sprintf(buf2, "0%d:", min);
    else
        sprintf(buf2, "%d:", min);
    if(sec < 10)
        sprintf(buf3, "0%f", sec);
    else
        sprintf(buf3, "%f", sec);
    strcat(r_eal, buf1);
    strcat(r_eal, buf2);
    strcat(r_eal, buf3);

}

void creat_name(int mv_count, char * r_eal){
    char buf[256];
    sprintf(buf, "./makemv111/seg%d.mp4", mv_count);
    strcat(r_eal, buf);
}


void zero_histo(){

    for(int i =0; i<256; i++){
        HistR_cur[i] = 0;
        HistG_cur[i] = 0;
        HistB_cur[i] = 0;
    }
}

void cur_frame(Mat image){
    zero_histo();
    for (int i = 0; i < image.rows; i++){
        for (int j = 0; j < image.cols; j++){

            Vec3b intensity = image.at<Vec3b>(Point(j, i));

            int Red = intensity.val[0];
            int Green = intensity.val[1];
            int Blue = intensity.val[2];

            HistR_cur[Red] = HistR_cur[Red]+1;
            HistB_cur[Blue] = HistB_cur[Blue]+1;
            HistG_cur[Green] = HistG_cur[Green]+1;


        }
    }
}

void cur_to_prev(int *prev, int *cur){

    for(int i = 0; i < 256; i++){

        prev[i] = cur[i];
    }
}

void diff_frame(){
    ret_R = 0;
    ret_G = 0;
    ret_B = 0;
    for(int i = 0; i < 256; i++){
        HistR[i] = abs(HistR_prev[i] - HistR_cur[i]);
        HistG[i] = abs(HistG_prev[i] - HistG_cur[i]);
        HistB[i] = abs(HistB_prev[i] - HistB_cur[i]);

        ret_R += HistR[i];
        ret_G += HistG[i];
        ret_B += HistB[i];

    }

}

int main() {
    int frame_count = 0 ;
    double div_frame_count ;
    VideoCapture vc;
    Mat frame;
    int result_histo; 
    char buf[256] = {0,};
    double sec = 0.00;
    double start =0.0;
    double end;
    bool gogo=0;
    char gogosing[256] = {0,};
    char mv_name[256] = {0,};
    char cut_start[256] = {0,};
    char cut_end[256] = {0,};
    int mv_count = 0;
    int how_many = 0;
    vc.open("/home/seungbu/workspace/hack_day/practice/testmv/hackday_mv_02.mp4");

    if (!vc.isOpened()) {
        return 0;

    }

    while (true) {
        vc >> frame;
        if (frame.empty()) {
            div_frame_count = frame_count / 24.0;
            char r_eal[256] = {0,};

            sec = div_frame_count;
            end = sec;

            sprintf(gogosing, "../ffmpeg-4.1-64bit-static/ffmpeg -v 1 -y -i ./testmv/hackday_mv_02.mp4 -ss "); 
            strcat(r_eal, gogosing);
            ret_time(start, r_eal);
            strcat(r_eal, " -to ");
            ret_time(end, r_eal);          
            strcat(r_eal, " -c:v libx264 -vf scale=640:360 -f mp4 ");
            creat_name(mv_count, r_eal);
            system(r_eal);
            mv_count++;
            start = end;
            printf("%s\n", r_eal);

            break;
        }
//        frame_count++;
//        continue;
        cur_frame(frame);
        if(how_many == 0){

            cur_to_prev(HistR_prev, HistR_cur);
            cur_to_prev(HistG_prev, HistG_cur);
            cur_to_prev(HistB_prev, HistB_cur);
            how_many++;
            continue;
        }
        diff_frame(); 

        frame_count ++;
        result_histo = ret_R + ret_B + ret_G;
        if(result_histo > 1000000){  
            div_frame_count = frame_count / 24.0;
            char r_eal[256] = {0,};

            sec = div_frame_count;
            end = sec;

            sprintf(gogosing, "../ffmpeg-4.1-64bit-static/ffmpeg -v 1 -y -i ./testmv/hackday_mv_02.mp4 -ss "); 
            strcat(r_eal, gogosing);
            ret_time(start, r_eal);
            strcat(r_eal, " -to ");
            ret_time(end, r_eal);          
            strcat(r_eal, " -c:v libx264 -vf scale=640:360 -f mp4 ");
            creat_name(mv_count, r_eal);
            system(r_eal);
            mv_count++;
            start = end;
            printf("%s\n", r_eal);
        }

        cur_to_prev(HistR_prev, HistR_cur);
        cur_to_prev(HistG_prev, HistG_cur);
        cur_to_prev(HistB_prev, HistB_cur);
        how_many++;

    }
    return 0;
}
