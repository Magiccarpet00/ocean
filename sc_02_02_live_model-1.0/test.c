#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265

typedef struct
{
    float z;
    float selfTime;
    int go;
}vec3_t;

#define TENSOR_SIZE 25
#define TENSOR_DEPTH 5
#define KERNEL_SIZE 3

static vec3_t tensor[TENSOR_SIZE][TENSOR_SIZE][TENSOR_DEPTH];
static vec3_t tmp_tensor[TENSOR_SIZE][TENSOR_SIZE][TENSOR_DEPTH];
static int nbWave = 0;

static float kernel[KERNEL_SIZE][KERNEL_SIZE] = {
    {0.2, 0.4, 0.2},
    {0.4, 0.0, 0.4},
    {0.2, 0.4, 0.2},
};

void drawTensor()
{
    for(int i = 0; i< TENSOR_SIZE; i++)
    {
        for(int j= 0; j< TENSOR_SIZE; j++)
        {
            float accZ = 0;
            for(int k = 0; k< TENSOR_DEPTH; k++)
            {   
                
                if(tensor[i][j][k].selfTime < 2 * PI && tensor[i][j][k].go)
                {
                    accZ +=  sinf(tensor[i][j][k].selfTime) * tensor[i][j][k].z;
                    tensor[i][j][k].selfTime += PI / 4.0f;
                }
            }

            // if(accZ > 0)
            //     printf("+", accZ);
            // else if(accZ < 0)
            //     printf("x", accZ);
            // else
            //     printf("-");



            if(accZ != 0)
                printf("[%.1f]", accZ*255);
            else
                printf("[---]");


        }
        printf("\n");
    }
    printf("\n");
}

void addPerturbation(int x, int y, float vel_z)
{
    tensor[x][y][nbWave].z = vel_z;
    tensor[x][y][nbWave].go = 1;
    nbWave++;
}

void convolution()
{
    for(int i = 0; i< TENSOR_SIZE; i++)
    {
        for(int j = 0; j< TENSOR_SIZE; j++)
        {
            for(int k = 0; k< TENSOR_DEPTH; k++)
            {
                tmp_tensor[i][j][k].z = tensor[i][j][k].z;
                tmp_tensor[i][j][k].go = tensor[i][j][k].go;
                tmp_tensor[i][j][k].selfTime = tensor[i][j][k].selfTime;

                //CONVOLUTION
                float accZ = 0.0f;
                for (int _i = -1; _i < 2; _i++)
                {
                    for (int _j = -1; _j < 2; _j++)
                    {
                        if(i + _i >= 0 && i + _i <TENSOR_SIZE &&
                            j + _j >= 0 && j + _j <TENSOR_SIZE)
                        {
                            accZ += tensor[i+_i][j+_j][k].z * kernel[_i+1][_j+1];
                        }
                    }
                }

                if(tensor[i][j][k].go == 0 && accZ > 0.0f)
                {
                    tensor[i][j][k].go = 1;
                    tmp_tensor[i][j][k].z = accZ;
                }
            }
        }
    }

    for(int i = 0; i< TENSOR_SIZE; i++)
        for(int j = 0; j< TENSOR_SIZE; j++)
            for(int k = 0; k< TENSOR_DEPTH; k++)
                tensor[i][j][k].z = tmp_tensor[i][j][k].z;

}

int main()
{
    //INIT
    for(int i = 0; i< TENSOR_SIZE; i++)
    {
        for(int j= 0; j< TENSOR_SIZE; j++)
        {
            for(int k = 0; k< TENSOR_DEPTH; k++)
            {
                tensor[i][j][k].z = 0.0f;
                tensor[i][j][k].selfTime = 0.0f;
                tensor[i][j][k].go = 0;
            }    
        }
    }

    drawTensor();
    addPerturbation(7,8,1);
    for (int i = 0; i < 12; i++)
    {
        drawTensor();
        convolution();
    }
    
    
    

}