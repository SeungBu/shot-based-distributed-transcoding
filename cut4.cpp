#include "../opencv-3.4.0/include/opencv2/opencv.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace cv;

#define threshold 1000000
#define fps 24.0

struct histogram_prev{

    int histB_prev[256];
    int histG_prev[256];
    int histR_prev[256];

};

struct histogram_cur{

    int histB_cur[256];
    int histG_cur[256];
    int histR_cur[256];

};

char* calctime_for_shot(double time, char * buf_for_calctime){

    int hour, min;
    int time_int = time;
    time = time - time_int;
    double sec;

    char hour_buf[256] = {0,};
    char min_buf[256] = {0,};
    char sec_buf[256] = {0,};
    
    min = time_int/60;
    hour = min/60;
    sec = time_int%60 + time;
    min = min%60;

    if(hour < 10)
        sprintf(hour_buf, "0%d:", hour );
    else
        sprintf(hour_buf, "%d:", hour);

    if(min < 10)
        sprintf(min_buf, "0%d:", min);
    else
        sprintf(min_buf, "%d:", min);
    
    if(sec < 10)
        sprintf(sec_buf, "0%f", sec);
    else
        sprintf(sec_buf, "%f", sec);

    strcat(buf_for_calctime, hour_buf);
    strcat(buf_for_calctime, min_buf);
    strcat(buf_for_calctime, sec_buf);
    
    return buf_for_calctime;
}

void get_cur_frame_hist(Mat image, struct histogram_cur * hist_cur){
    
    memset(hist_cur, 0, sizeof(struct histogram_cur));

    for (int i = 0; i < image.rows; i++){
        for (int j = 0; j < image.cols; j++){

            Vec3b intensity = image.at<Vec3b>(Point(j, i));

            int Red = intensity.val[0];
            int Green = intensity.val[1];
            int Blue = intensity.val[2];

            hist_cur->histR_cur[Red] = hist_cur->histR_cur[Red]+1;
            hist_cur->histB_cur[Blue] = hist_cur->histB_cur[Blue]+1;
            hist_cur->histG_cur[Green] = hist_cur->histG_cur[Green]+1;
        }
    }
}

int diff_frame_hist(struct histogram_prev* hist_prev, struct histogram_cur* hist_cur){
    
    int total_R = 0;
    int total_G = 0;
    int total_B = 0;

    for(int i = 0; i < 256; i++){
        total_R += abs(hist_prev->histR_prev[i] - hist_cur->histR_cur[i]);
        total_G += abs(hist_prev->histG_prev[i] - hist_cur->histG_cur[i]);
        total_B += abs(hist_prev->histB_prev[i] - hist_cur->histB_cur[i]);
    }

    return total_R + total_G + total_B;
}

void creat_segment_list(int dir_num, int seg_count){
    
    int i = 0;
    char txt_open[256] = {0,};

    sprintf(txt_open, "./segment_video%d/segment_list.txt", dir_num);

    FILE *fp = fopen(txt_open, "w");
    
    for( i = 0 ; i <= seg_count ; i++ )        
        fprintf(fp, "file segment%d.mp4\n", i);
    
    fclose(fp);
}

int main() {
    
    int frame_count = 0 ;
    int sum_histRGB = 0; 
    int dir_num = 0;
    int seg_count = 0;

    double start_time = 0.0;
    double end_time = 0.0;

    char getline_playlist[256] = {0,};
    char shot_command[256] = {0,};
    char merge_command[256] = {0,};
    char buf_for_start_calctime[256] = {0,};
    char buf_for_end_calctime[256] = {0,};
    char mkdir_for_shotvideo[256] = {0,};

    char * shot_start_time = NULL;
    char * shot_end_time = NULL;

    VideoCapture vc;
    Mat frame;

    struct histogram_prev *hist_prev = (struct histogram_prev*)malloc(sizeof(struct histogram_prev));
    struct histogram_cur *hist_cur = (struct histogram_cur*)malloc(sizeof(struct histogram_cur));
    
    ifstream fp_for_playlist("play_list.txt");

    while(!fp_for_playlist.eof()){ 
        
        frame_count = 0;
        seg_count =0;
        start_time = 0.0;
        end_time = 0.0;        

        sprintf(mkdir_for_shotvideo, "mkdir segment_video%d", dir_num);
        system(mkdir_for_shotvideo);

        fp_for_playlist.getline(getline_playlist, 256);
        vc.open(getline_playlist);

        if (!vc.isOpened()) {
            return 0;
        }

        cout << "end_time : " << end_time << " dir_num = " << dir_num << endl;

        while (true) {
        
            memset(buf_for_start_calctime, 0, sizeof(buf_for_start_calctime));
            memset(buf_for_end_calctime, 0, sizeof(buf_for_end_calctime));

            vc >> frame;
            
            if (frame.empty()) {
                
                end_time = frame_count / 24.0;

                shot_start_time = calctime_for_shot(start_time, buf_for_start_calctime);
                shot_end_time = calctime_for_shot(end_time, buf_for_end_calctime);          
                sprintf(shot_command, "../ffmpeg-4.1-64bit-static/ffmpeg -v 1 -y -i ./%s -ss %s -to %s -c:v libx264 -vf scale=256:144 -f mp4 ./segment_video%d/segment%d.mp4"
                        , getline_playlist, shot_start_time, shot_end_time, dir_num, seg_count);
                system(shot_command);
                
                printf("%s\n", shot_command);

                break;
            }

            get_cur_frame_hist(frame, hist_cur);

            if(frame_count == 0){
                memcpy(hist_prev, hist_cur, sizeof(struct histogram_cur));
                frame_count ++;
                continue;
            }
            
            sum_histRGB = diff_frame_hist(hist_prev, hist_cur); 

            if(sum_histRGB > threshold){  

                end_time = frame_count / fps;

                shot_start_time = calctime_for_shot(start_time, buf_for_start_calctime);
                shot_end_time = calctime_for_shot(end_time, buf_for_end_calctime);          
                sprintf(shot_command, "../ffmpeg-4.1-64bit-static/ffmpeg -v 1 -y -i ./%s -ss %s -to %s -c:v libx264 -vf scale=256:144 -f mp4 ./segment_video%d/segment%d.mp4"
                        , getline_playlist, shot_start_time, shot_end_time, dir_num, seg_count);
                system(shot_command);

                seg_count++;
                start_time = end_time;
                printf("%s\n", shot_command);

            }

            memcpy(hist_prev, hist_cur, sizeof(struct histogram_cur));

            frame_count ++;
        }

        creat_segment_list(dir_num, seg_count);
        
        sprintf(merge_command, "../ffmpeg-4.1-64bit-static/ffmpeg -f concat -i ./segment_video%d/segment_list.txt -c copy ./merge_video/merge%d.mp4", dir_num, dir_num);
        system(merge_command); 
        
        dir_num++;
        
        vc.release();
    }
    
    fp_for_playlist.close();
    return 0;
}
