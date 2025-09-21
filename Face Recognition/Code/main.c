#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <conio.h> 

#define NUM_IMAGES 10
#define TH_val 50 
#pragma pack(push, 1) 

typedef struct {
    unsigned short type;      //Loại ảnh
    unsigned int size;       //Kích thước ảnh
    unsigned short reserved1; 
    unsigned short reserved2; 
    unsigned int offset;      // Địa chỉ đầu
} BMPHeader;
typedef struct {
    unsigned int size;          
    int w;                  
    int h;                
    unsigned short planes;      
    unsigned short bitsPerPixel; 
    unsigned int compression;    // Phương pháp nén
    unsigned int imageSize;      // Kích thước hình ảnh
    int xPixelsPerMeter;        
    int yPixelsPerMeter;        
    unsigned int colorsUsed;      // Số màu
    unsigned int importantColors;
} BMPInfoHeader;
#pragma pack(pop)

unsigned char* readBMPimg(const char *filename, int *w, int *h) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Không thể mở file %s\n", filename);
        exit(1);
    }
    BMPHeader bmpHeader;
    BMPInfoHeader bmpInfoHeader;
    fread(&bmpHeader, sizeof(BMPHeader), 1, fp);
    fread(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, fp);
    if (bmpHeader.type != 0x4D42) {     //Mã ASCII hexa của M là 4D, của B là 42
        printf("Không phải file BMP\n");
        fclose(fp);
        exit(1);
    }
    *w = bmpInfoHeader.w;
    *h = bmpInfoHeader.h;
    unsigned char *image = (unsigned char *)malloc((*w) * (*h) * 3); // 3 bytes per pixel (RGB)
    fseek(fp, bmpHeader.offset, SEEK_SET); 
    fread(image, 3, (*w) * (*h), fp);
    fclose(fp);
    return image;
}

unsigned char* convertToGray(unsigned char *rgbImage, int w, int h) {
    unsigned char *grayImage = (unsigned char *)malloc(w * h * sizeof(unsigned char));
    for (int i = 0; i < w * h; i++) {
        //  Y' = 0.299 * R + 0.587 * G + 0.114 * B
        unsigned char r = rgbImage[i * 3];
        unsigned char g = rgbImage[i * 3 + 1];
        unsigned char b = rgbImage[i * 3 + 2];
        grayImage[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
    return grayImage;
}

void writeGrayBMPimg(const char *filename, unsigned char *image, int w, int h) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Không thể mở file %s để ghi\n", filename);
        exit(1);
    }
    BMPHeader bmpHeader;
    BMPInfoHeader bmpInfoHeader;
    bmpHeader.type = 0x4D42; // 'BM'
    bmpHeader.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + w * h;
    bmpHeader.reserved1 = 0;
    bmpHeader.reserved2 = 0;
    bmpHeader.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
    bmpInfoHeader.size = sizeof(BMPInfoHeader);
    bmpInfoHeader.w = w;
    bmpInfoHeader.h = h;
    bmpInfoHeader.planes = 1;
    bmpInfoHeader.bitsPerPixel = 8; 
    bmpInfoHeader.compression = 0;
    bmpInfoHeader.imageSize = w * h;
    bmpInfoHeader.xPixelsPerMeter = 0;
    bmpInfoHeader.yPixelsPerMeter = 0;
    bmpInfoHeader.colorsUsed = 256; //2^8 bit = 256 
    bmpInfoHeader.importantColors = 256;
    fwrite(&bmpHeader, sizeof(BMPHeader), 1, fp);
    fwrite(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, fp);
    for (int i = 0; i < 256; i++) {
        unsigned char color[4] = {i, i, i, 0};
        fwrite(color, sizeof(color), 1, fp);
    }
    fwrite(image, 1, w * h, fp); 
    fclose(fp);
}

int applyKernel(unsigned char *image, int w, int h, int x, int y, int kernel[5][5]) {
    int I = 0;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            //Sao chép giá trị biên để tính phần rìa ảnh
            int newY = y + i;
            int newX = x + j;
            if (newY < 0) newY = 0;           
            if (newY >= h) newY = h - 1;      
            if (newX < 0) newX = 0;           
            if (newX >= w) newX = w - 1;      
            I += abs(image[newY * w + newX] * kernel[i+2][j+2]);       //Định nghĩa lại vị trí từ kernel
        }
    }
    return I;
}

void Kernel(unsigned char *image, int w, int h, int *Fh, int *Fp, int *Fv, int *Fm) {
    int Kh[5][5] = {{0, 0, 0, 0, 0},  {1, 1, 1, 1, 1},   {0, 0, 0, 0, 0},  {-1, -1, -1, -1, -1}, {0, 0, 0, 0, 0} };
    int Kp[5][5] = {{0, 0, 0, 1, 0},  {0, 1, 1, 0, -1},  {0, 1, 0, -1, 0}, {1, 0, -1, -1, 0},    {0, -1, 0, 0, 0}};
    int Kv[5][5] = {{0, 1, 0, -1, 0}, {0, 1, 0, -1, 0},  {0, 1, 0, -1, 0}, {0, 1, 0, -1, 0},     {0, 1, 0, -1, 0}};
    int Km[5][5] = {{0, -1, 0, 0, 0}, {1, 0, -1, -1, 0}, {0, 1, 0, -1, 0}, {0, 1, 1, 0, -1},     {0, 0, 0, 1, 0} };
    int *Ih = (int *)calloc(w * h, sizeof(int));
    int *Ip = (int *)calloc(w * h, sizeof(int));
    int *Iv = (int *)calloc(w * h, sizeof(int));
    int *Im = (int *)calloc(w * h, sizeof(int));
    for (int y = 0; y < h ; y++) {
        for (int x = 0; x < w ; x++) {
            Ih[y * w + x] = applyKernel(image, w, h, x, y, Kh);
            Ip[y * w + x] = applyKernel(image, w, h, x, y, Kp);
            Iv[y * w + x] = applyKernel(image, w, h, x, y, Kv);
            Im[y * w + x] = applyKernel(image, w, h, x, y, Km);

            // Tìm max I(x,y)(Idh, Idp, Idv, Idm)
            int max = Ih[y * w + x];
            if (Ip[y * w + x] > max) max = Ip[y * w + x];
            else if (Iv[y * w + x] > max) max = Iv[y * w + x];
            else if (Im[y * w + x] > max) max = Im[y * w + x];
            // Gộp công thức 3 và 7 so sánh và tạo direction map Fh, Fp, Fv, Fm 
            if (Ih[y * w + x] == max && Ih[y * w + x] > TH_val) {
                Fh[y * w + x] = 1;  Fp[y * w + x] = 0 ;
                Fv[y * w + x] = 0;  Fm[y * w + x] = 0 ;               
            } else if (Ip[y * w + x] == max && Ip[y * w + x] > TH_val) {
                Fp[y * w + x] = 1;  Fh[y * w + x] = 0 ;
                Fv[y * w + x] = 0;  Fm[y * w + x] = 0 ;  
            } else if (Iv[y * w + x] == max && Iv[y * w + x] > TH_val) {
                Fv[y * w + x] = 1;  Fh[y * w + x] = 0 ;
                Fp[y * w + x] = 0;  Fm[y * w + x] = 0 ;  
            } else if (Im[y * w + x] == max && Im[y * w + x] > TH_val) {
                Fm[y * w + x] = 1;  Fh[y * w + x] = 0 ;
                Fp[y * w + x] = 0;  Fv[y * w + x] = 0 ;  
            }
        }
    }
}

void APEDfunction(int *Fh, int *Fp, int *Fv, int *Fm, int *H, int *P, int *V, int *M){
    for (int a = 0; a < 4; a++) {
        for (int b = 0; b < 4; b++) {
            int Htmp = 0, Ptmp = 0,  Vtmp = 0, Mtmp = 0 ;
            for (int i = 0; i <  16; i++) {
                for (int j = 0; j < 16; j++) {
                    Htmp += Fh[(16 * b + j) + (16 * a + i)] ; 
                    Ptmp += Fp[(16 * b + j) + (16 * a + i)] ;
                    Vtmp += Fv[(16 * b + j) + (16 * a + i)] ; 
                    Mtmp += Fm[(16 * b + j) + (16 * a + i)] ;
                }
            }
            H[a + 4 * b] = Htmp ;  P[a + 4 * b] = Ptmp ;
            V[a + 4 * b] = Vtmp ;  M[a + 4 * b] = Mtmp ;
        }
    }
}

int main() {
    char directory[100];
    printf("Nhập đường dẫn đến thư mục chứa 10 ảnh BMP: ");
    scanf("%s", directory);
    
    char filenames[NUM_IMAGES][150];
    struct dirent *de;
    DIR *dr = opendir(directory);
    int imageCount = 0;
    
    if (dr == NULL) {
        printf("Không thể mở thư mục: %s\n", directory);
        return 1;
    }
    
    while ((de = readdir(dr)) != NULL && imageCount < NUM_IMAGES) {
        if (strstr(de->d_name, ".bmp") != NULL) {
            snprintf(filenames[imageCount], sizeof(filenames[imageCount]), "%s/%s", directory, de->d_name);
            imageCount++;
        }
    }
    
    closedir(dr);
    printf("Số lượng ảnh BMP tìm thấy: %d\n", imageCount);
    
    // Khởi tạo các biến để lưu thông tin ảnh
    int H[NUM_IMAGES][16], P[NUM_IMAGES][16], V[NUM_IMAGES][16], M[NUM_IMAGES][16];
    int *Fh[NUM_IMAGES], *Fp[NUM_IMAGES], *Fv[NUM_IMAGES], *Fm[NUM_IMAGES];
    unsigned char *rgbImage[NUM_IMAGES], *grayImage[NUM_IMAGES];
    int widths[NUM_IMAGES], heights[NUM_IMAGES];

    // Đọc và xử lý các ảnh trong thư mục đã chỉ định
    for (int i = 0; i < imageCount; i++) {
        rgbImage[i] = readBMPimg(filenames[i], &widths[i], &heights[i]);
        grayImage[i] = convertToGray(rgbImage[i], widths[i], heights[i]);
        Fh[i] = (int *)calloc(widths[i] * heights[i], sizeof(int));
        Fp[i] = (int *)calloc(widths[i] * heights[i], sizeof(int));
        Fv[i] = (int *)calloc(widths[i] * heights[i], sizeof(int));
        Fm[i] = (int *)calloc(widths[i] * heights[i], sizeof(int));

        Kernel(grayImage[i], widths[i], heights[i], Fh[i], Fp[i], Fv[i], Fm[i]);
        APEDfunction(Fh[i], Fp[i], Fv[i], Fm[i], H[i], P[i], V[i], M[i]);
        free(grayImage[i]);
        free(rgbImage[i]);
    }

    printf("Nhấn phím bất kỳ để bắt đầu so sánh ảnh test (nhấn 'Esc' để thoát):\n");

    while (1) {
        if (_kbhit()) {
            char ch = _getch(); 
            if (ch == 27) { 
                break; 
            }

            char inputFilename[150];
            printf("Nhập tên file ảnh cần so sánh (bao gồm đường dẫn): ");
            scanf("%s", inputFilename);

            int testWidth, testHeight;
            unsigned char *testImg = readBMPimg(inputFilename, &testWidth, &testHeight);
            unsigned char *testgrayImg = convertToGray(testImg, testWidth, testHeight);
            
            int *inputFh = (int *)calloc(testWidth * testHeight, sizeof(int));
            int *inputFp = (int *)calloc(testWidth * testHeight, sizeof(int));
            int *inputFv = (int *)calloc(testWidth * testHeight, sizeof(int));
            int *inputFm = (int *)calloc(testWidth * testHeight, sizeof(int));
            
            Kernel(testgrayImg, testWidth, testHeight, inputFh, inputFp, inputFv, inputFm);
            int testH[16], testP[16], testV[16], testM[16];
            APEDfunction(inputFh, inputFp, inputFv, inputFm, testH, testP, testV, testM);

            int matchIndex = -1;
            int mindiff = 1e4; 
            for (int i = 0; i < imageCount; i++) {
                int currentDiff = 0;  // Khởi tạo độ khác biệt cho ảnh thứ i
                for (int j = 0; j < 16; j++) {
                    currentDiff += abs(testH[j] - H[i][j]);
                    currentDiff += abs(testP[j] - P[i][j]);
                    currentDiff += abs(testV[j] - V[i][j]);
                    currentDiff += abs(testM[j] - M[i][j]);
                }
                if (currentDiff < mindiff) {
                    mindiff = currentDiff;
                    matchIndex = i;
                }  
            }   

            if (matchIndex != -1) {
                printf("Ảnh được so sánh là gần nhất với: %s\n", filenames[matchIndex]);
            } else {
                printf("Không tìm thấy ảnh phù hợp.\n");
            }

            free(testgrayImg);
            free(testImg);
            free(inputFh);
            free(inputFp);
            free(inputFv);
            free(inputFm);
        }
    }
    for (int i = 0; i < imageCount; i++) {
        free(Fh[i]);
        free(Fp[i]);
        free(Fv[i]);
        free(Fm[i]);
    }
    return 0;
}