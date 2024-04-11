#include<iostream>
#include<fstream>
#include<string>
#include<vector> 
#include<unistd.h>
using namespace std;

/**
 * All the below values are based on this paper http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
 */
#define HEADER_SIZE 14
#define INFO_HEADER 40
/** I know this because the run time of the video is 3:39 and it's fps is 30
 * ((3 * 60) + 39) * 30 == 6570
 * 
 * but the ffmpeg will generating extra 2 frames at the end
 * it is not a big deal.
*/
#define FRAMES 6570
#define BYTE 8
#define H_BYTE 4
#define COLORS_PER_PIXEL 3 // R,G,B
#define FPS 30
// reading values byte by byte and char is exactly a byte;
#define BYTE_READ file.read((char*) &byte, sizeof(byte))

struct Pixel {
    unsigned int r;
    unsigned int g;
    unsigned int b;
};

struct Frame {
    unsigned int width;
    unsigned int height;
    vector<vector<Pixel> > pixels;
};

Frame readFrame(int frame)
{
    string currentFrame = to_string(frame);
    if(frame < 1000)
        currentFrame = "0" + currentFrame;
    if(frame < 100)
        currentFrame = "0" + currentFrame;
    if(frame < 10)
        currentFrame = "0" + currentFrame;

    currentFrame = "./frames/" + currentFrame + ".bmp";
    ifstream file(currentFrame, ios::binary);
    unsigned char byte;
    Frame f;

    //Header & InfoHeader
    vector<unsigned char> header;
    for(int i=0;i<HEADER_SIZE + INFO_HEADER;i++){
        BYTE_READ;
        header.push_back(byte);
    }

    //Width
    int width_offset = HEADER_SIZE + H_BYTE;
    unsigned int width = 0;
    for(int i=H_BYTE-1;i>=0;i--){
        unsigned char current_byte = header[width_offset + i];
        width = width << BYTE;
        width = width | (int)current_byte;
    }
    f.width = width;

    //Height
    int height_offset = width_offset + H_BYTE;
    unsigned int height = 0;
    for(int i=H_BYTE-1;i>=0;i--){
        unsigned char current_byte = header[height_offset + i];
        height = height << BYTE;
        height = height | (int)current_byte;
    }
    f.height = height;

    //Reading Pixels
    for(int i=0;i<height;i++) {
        vector<Pixel> row;
        for(int j=0;j<width * COLORS_PER_PIXEL;j+=COLORS_PER_PIXEL) {
            Pixel p;
            BYTE_READ;
            p.b = (int)byte;
            BYTE_READ;
            p.g = (int)byte;
            BYTE_READ;
            p.r = (int)byte;
            row.push_back(p);
        }
        f.pixels.push_back(row);
    }

    reverse(f.pixels.begin(), f.pixels.end());
    return f;
}

/** 
 * As the original frame from the video may be too long to print,
 * based on the ${pixels_per_block} i am downscaling then to specific
 * blocks and averaging all the pixel colors in that specific block and
 * showing it as pixel.
 * 
*/
void renderFrame(Frame f, int pixels_per_block = 10) {
    string currentFrame = "";
    for(int i=0;i<f.pixels.size();i+=pixels_per_block){
        for(int j=0;j<f.pixels[i].size();j+=pixels_per_block){
            int r = 0;
            int g = 0;
            int b = 0;

            // Block
            for(int x=0;x<pixels_per_block;x++){
                for(int y=0;y<pixels_per_block;y++){
                    Pixel p = f.pixels[i + x][j + y];
                    r += p.r;
                    g += p.g;
                    b += p.b;
                }
            }
            r /= (pixels_per_block * pixels_per_block);
            g /= (pixels_per_block * pixels_per_block);
            b /= (pixels_per_block * pixels_per_block);

            if(r > 0 || g > 0 || b > 0)
                currentFrame+="$$"; // White Pixel
            else 
                currentFrame+=".."; // Black Pixel
        }
        currentFrame+="\n";
    }
    cout<<currentFrame;
}

int main() {
    std::ios::sync_with_stdio(false);
    for(int i=1;i<=FRAMES;i++){
        Frame f = readFrame(i);
        if(i == 1){
            cout<< "press enter key to start";
            cin.get();
        }
        renderFrame(f, 9);
        /** because i am computing and rendering the frame at the same time
         * there is a delay from original video. so by multiply this
         * constant(3.18) with FPS fixed that issue.
        */
        usleep(1000000 / (FPS * 3.18));
    }
    return 0;
}