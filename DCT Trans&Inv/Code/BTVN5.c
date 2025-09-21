#include <stdio.h>
#include <math.h>
#include <string.h>
#define N 8

void scaleValue(int In[N][N], int Out[N][N], int Scale) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            if (Scale) {
                Out[x][y] = In[x][y] - 128;
            } else {
                Out[x][y] = In[x][y] + 128;
                printf("%d ", Out[x][y]) ;
            }
        }
        printf("\n") ;
    }
}
void DCT_trans(int input[N][N], double output[N][N]) {
    double Cu, Cv, F;
    double PI = 3.14159265358979323846;
    for (int u = 0; u < N; u++) {
        for (int v = 0; v < N; v++) {
            if (u == 0) Cu = 1.0 / sqrt(2); else Cu = 1.0;
            if (v == 0) Cv = 1.0 / sqrt(2); else Cv = 1.0;
            F = 0.0;
            for (int x = 0; x < N; x++) {
                for (int y = 0; y < N; y++) {
                    F += input[x][y] * cos(((2 * x + 1) * u * PI) / (2.0 * N)) * cos(((2 * y + 1) * v * PI) / (2.0 * N));
                }
            }
            output[u][v] = 0.25 * Cu * Cv * F;
            printf("%.1f ", output[u][v]) ;
        }
        printf("\n") ;
    }
}
void Quantize(double DCTcoeff[N][N], int QuanTable[N][N], int Result[N][N]) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            Result[x][y] = (int)(round(DCTcoeff[x][y] / QuanTable[x][y])) ;
            printf("%d ", Result[x][y]);
        }
        printf("\n");
    }
}
void Zigzag(int QuantizeMatrix[N][N], int MatrixVal[N * N]) {
    int x = 0, y = 0, index = 0;
    int up = 1; // Điều hướng di chuyển: 1 là lên trên, -1 là xuống dưới

    for (int i = 0; i < N * N; i++) {
        MatrixVal[index] = QuantizeMatrix[x][y] ;
        index++;
        printf("%d ", QuantizeMatrix[x][y]);

        if (up) {
            if (y == N - 1) {
                x++;     // Nếu đã đến cuối cột, chuyển sang hàng tiếp theo
                up = 0;  // Đổi hướng
            } else if (x == 0) {
                y++;     // Nếu đã đến đầu hàng, chuyển sang cột tiếp theo
                up = 0;  // Đổi hướng
            } else {
                x--; y++; // Di chuyển chéo lên trên
            }
        } else {
            if (x == N - 1) {
                y++;     // Nếu đã đến cuối hàng, chuyển sang cột tiếp theo
                up = 1;  // Đổi hướng
            } else if (y == 0) {
                x++;     // Nếu đã đến đầu cột, chuyển sang hàng tiếp theo
                up = 1;  // Đổi hướng
            } else {
                x++; y--; // Di chuyển chéo xuống dưới
            }
        }
    }
    printf("\n");
}

int SizeCal(int value) {
    if (value == 0) return 0;  
    return (int)log2(abs(value)) + 1;  
}
//VLE: Variable-Length Entropy Coding
void BaselineEncode(int DCpre, int input[N * N], int *VLEcode, int *VLEsize) {
    int VLE_index = 0;
    int DCdiff = abs(input[0] - DCpre) ;
    int DCSize = SizeCal(DCdiff) ;
    VLEcode[VLE_index++] = DCSize ;
    VLEcode[VLE_index++] = DCdiff ;
    int i = 1; 
    int ZeroRunLength = 0;
    while (i < N * N) {
        if (input[i] == 0) {
            ZeroRunLength++;  // Tăng độ dài chuỗi số 0
        } else {
            int amp = input[i];
            int size = SizeCal(amp);  
            VLEcode[VLE_index++] = ZeroRunLength;  
            VLEcode[VLE_index++] = size;          
            VLEcode[VLE_index++] = amp;       
            ZeroRunLength = 0;  
        }
        i++;
    }
    VLEcode[VLE_index++] = 256;  
    *VLEsize = VLE_index;  
}

void printVLE(int *VLEcode, int VLEsize) {
    printf("(DC Size, DC Amplitude): (%d, %d)\n", VLEcode[0], VLEcode[1]); 
    printf("(Zero-run length, Size)(Amplitude): \n");
    for (int i = 2; i < VLEsize; i += 3) {  
        if (VLEcode[i] == 256) {  
            printf("(0, 0) (EOB)\n");
            break;
        }
        printf("(%d, %d)(%d)\n", VLEcode[i], VLEcode[i + 1], VLEcode[i + 2]);
    }
}

const char* Symbol1Code(int zero_run, int size) {
    if (size == 1) {
        if (zero_run == 0) return "00 "; //(0,1)
        if (zero_run == 2) return "11100 ";  // (2,1)
    }
    if (zero_run == 1 && size == 2) return "11011 " ;   //(1,2)
    return "011 "; // (2) của DC
}

const char* Symbol2Code(int amp) {
    if (amp == 2) return "011 ";
    if (amp == 3) return "11 " ;
    if (amp == -2) return "01 " ;
    if (amp == -1) return "0 "   ;
}

void HuffmanEncode(int *VLEcode, int VLEsize, char *BitStream) {
    BitStream[0] = '\0'; 
    int DCSize = VLEcode[0];
    int DCAmp = VLEcode[1];
    const char* dcSize = Symbol1Code(DCSize, DCAmp);
    strcat(BitStream, dcSize); 
    const char* dcAmp = Symbol2Code(DCAmp);
    strcat(BitStream, dcAmp); 
    
    for (int i = 2; i < VLEsize; i += 3) {
        if (VLEcode[i] == 256) { 
            strcat(BitStream, "1010 ");
            break;
        }
        int zeroRunLength = VLEcode[i];
        int size = VLEcode[i + 1];
        int amp = VLEcode[i + 2];
        const char* code = Symbol1Code(zeroRunLength,size);
        strcat(BitStream, code); 
        const char* AmpCode = Symbol2Code(amp);
        strcat(BitStream, AmpCode);
    }
    printf("%s", BitStream) ;
}
void HuffmanDecode(const char *encodedString, int *VLEoutput, int *VLEsize) {
    char *token;
    char encodedCopy[100];
    strcpy(encodedCopy, encodedString);  // Sao chép chuỗi để sử dụng strtok
    int VLE_index = 0;

    token = strtok(encodedCopy, " ");
    while (token != NULL) {
        if (strcmp(token, "011") == 0) {  // DC size = 2
            VLEoutput[VLE_index++] = 2;
        } else if (strcmp(token, "11") == 0) {  // DC amplitude = 3
            VLEoutput[VLE_index++] = 3;
        } else if (strcmp(token, "01") == 0) {  // Amp = -2
            VLEoutput[VLE_index++] = -2;
        } else if (strcmp(token, "0") == 0) {  // Amp = -1
            VLEoutput[VLE_index++] = -1;
        } else if (strcmp(token, "00") == 0) {  // (0,1)
            VLEoutput[VLE_index++] = 0;  
            VLEoutput[VLE_index++] = 1;  
        } else if (strcmp(token, "11011") == 0) {  // (1,2)
            VLEoutput[VLE_index++] = 1;  
            VLEoutput[VLE_index++] = 2;  
        } else if (strcmp(token, "11100") == 0) {  // (2,1)
            VLEoutput[VLE_index++] = 2;  
            VLEoutput[VLE_index++] = 1;
        } else if (strcmp(token, "1010") == 0) {  // (0,0) EOB
            VLEoutput[VLE_index++] = 0;
            VLEoutput[VLE_index++] = 0;  
            break;  
        } else {
            printf("Token không hợp lệ: %s\n", token);
        }
        token = strtok(NULL, " ");  
    }
    *VLEsize = VLE_index;  
    for  (int i = 0; i < VLE_index; i++) {
        printf("%d ", VLEoutput[i]) ;
    }
}

void VLEtoMatrix(int DCpre, int VLEcode[], int VLEsize, int MatrixfromVLE[N][N]) {
    int zigzag[N * N] = {0};  //Vecto lưu trữ tạm các giá trị chuyển đổi trước khi chuyển sang dạng ma trận
    int zigzagIndex = 0;  
    int DCcur = DCpre + VLEcode[1];  
    zigzag[zigzagIndex++] = DCcur;  

    for (int i = 2; i < VLEsize; i += 3) {
        int zeroRun = VLEcode[i];  // Số lượng số 0
        int amp = VLEcode[i + 2];  // Giá trị amp
        if (zeroRun == 0 && amp == 0) {
            break;  
        }
        for (int j = 0; j < zeroRun; j++) {
            zigzag[zigzagIndex++] = 0;
        }
        zigzag[zigzagIndex++] = amp;
    }
    while (zigzagIndex < N * N) {
        zigzag[zigzagIndex++] = 0;
    }
    int x = 0, y = 0, index = 0;
    int up = 1;  // Điều hướng di chuyển: 1 là lên trên, -1 là xuống dưới

    for (int i = 0; i < N * N; i++) {
        MatrixfromVLE[x][y] = zigzag[index++];
        if (up) {
            if (y == N - 1) {
                x++;
                up = 0;
            } else if (x == 0) {
                y++;
                up = 0;
            } else {
                x--; y++;
            }
        } else {
            if (x == N - 1) {
                y++;
                up = 1;
            } else if (y == 0) {
                x++;
                up = 1;
            } else {
                x++; y--;
            }
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%4d", MatrixfromVLE[i][j]);
        }
        printf("\n");
    }
}
void deQuantize(int norCoeff[N][N], int QuanTable[N][N], int Result[N][N]) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            Result[x][y] = norCoeff[x][y] * QuanTable[x][y] ;
            printf("%d ", Result[x][y]);
        }
        printf("\n");
    }
}
void INVDCT_trans(int input[N][N], int output[N][N]) {
    double Cu, Cv, inv_F;
    double PI = 3.14159265358979323846;

    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            inv_F = 0.0;
            for (int u = 0; u < N; u++) {
                for (int v = 0; v < N; v++) {
                    if (u == 0) Cu = 1.0 / sqrt(2); else Cu = 1.0;
                    if (v == 0) Cv = 1.0 / sqrt(2); else Cv = 1.0;
                    inv_F += Cu * Cv * input[u][v] * cos((2 * x + 1) * u * PI / (2.0 * N)) * cos((2 * y + 1) * v * PI / (2.0 * N));
                }
            }
            output[x][y] = (int)round(0.25 * inv_F);
        }
    }
}

int main() {
    //Figure 10a
    int IN_img[N][N] = {
        {139, 144, 149, 153, 155, 155, 155, 155},
        {144, 151, 153, 156, 159, 156, 156, 156},
        {150, 155, 160, 163, 158, 156, 156, 156},
        {159, 161, 162, 160, 160, 159, 159, 159},
        {159, 160, 161, 162, 162, 155, 155, 155},
        {161, 161, 161, 161, 160, 157, 157, 157},
        {162, 162, 161, 163, 162, 157, 157, 157},
        {162, 162, 161, 161, 163, 158, 158, 158}
    };
    //Figure 10c
    int QuanTable[N][N] = {
        {16, 11, 10, 16, 24, 40, 51, 61},
        {12, 12, 14, 19, 26, 58, 60, 55},
        {14, 13, 16, 24, 40, 57, 69, 56},
        {15, 17, 22, 29, 51, 87, 80, 62},
        {18, 22, 37, 56, 68, 109, 103, 77},
        {24, 35, 55, 64, 81, 104, 113, 92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100,  103, 99}
    } ;
    int QuantCoeff[N][N], denorQuantCoeff[N][N],RecontImg[N][N], LinearString[N * N];
    int PreAmp = 12, VLEcode[N * 3], VLEsize = 0, MatrixVLE[N][N] = {0};
    double DCT_img[N][N] ;
    scaleValue(IN_img, IN_img,1);
    printf("Ma trận sau khi khai triển DCT (Figure 10b):\n");
    DCT_trans(IN_img, DCT_img);
    printf("Ma trận sau khi lượng tử hóa (Figure 10d):\n");
    Quantize(DCT_img, QuanTable, QuantCoeff);
    printf("Chuỗi tuyến tính qua đọc Zig-zag:\n");
    Zigzag(QuantCoeff, LinearString);
    BaselineEncode(PreAmp,LinearString, VLEcode, &VLEsize);
    printVLE(VLEcode, VLEsize);
    char Huffmanstr[N * N];
    printf("Kết quả mã hóa Huffman:");
    HuffmanEncode(VLEcode, VLEsize, Huffmanstr);
    printf("\nKết quả giải mã Huffman: ");
    HuffmanDecode(Huffmanstr, VLEcode, &VLEsize) ;
    printf("\nKết quả chuyển từ chuỗi giải mã hóa Huffman sang ma trận: \n");
    VLEtoMatrix(PreAmp, VLEcode, VLEsize, MatrixVLE) ;
    printf("Ma trận giải mã từ VLE sau lượng tử hóa ngược (Figure 10e):\n");
    deQuantize(MatrixVLE, QuanTable, denorQuantCoeff) ;
    printf("Ma trận sau tái cấu trúc (Figure 10f): \n");
    INVDCT_trans(denorQuantCoeff, RecontImg);
    scaleValue(RecontImg, RecontImg, 0) ;
    return 0;
} 