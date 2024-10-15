#include <stdio.h>
#include <stdlib.h>
#include "algorithm.h"

/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t Greedy(const InputParam *input, OutputParam *output)
{
    int32_t ret;

    // /* 算法示例：先入先出算法 */
    // output->len = input->ioVec.len;
    // for (uint32_t i = 0; i < output->len; i++) {
    //     output->sequence[i] = input->ioVec.ioArray[i].id;
    // }

    // /* 调用公共函数示例：调用电机寻址、带体磨损、电机磨损函数 */
    // HeadInfo start = {input->ioVec.ioArray[0].wrap, input->ioVec.ioArray[0].endLpos, HEAD_RW};
    // HeadInfo end = {input->ioVec.ioArray[1].wrap, input->ioVec.ioArray[1].endLpos, HEAD_RW};
    // int32_t seekT = 0;
    // int32_t beltW = 0;
    // int32_t motorW = 0;
    //    for (uint32_t i = 0; i < 10000; i++) {
    //        seekT = SeekTimeCalculate(&start, &end);
    //        beltW = BeltWearTimes(&start, &end, NULL);
    //        motorW = MotorWearTimes(&start, &end);
    //    }

    // /* 调用公共函数示例：调用IO读写时间函数 */
    // uint32_t rwT = ReadTimeCalculate(abs(input->ioVec.ioArray[0].endLpos - input->ioVec.ioArray[0].startLpos));
    output->len = input->ioVec.len;
    output->sequence = (uint32_t *)malloc(output->len * sizeof(uint32_t));
    if (output->sequence == NULL) {
        return RETURN_ERROR;
    }

    // 初始化已处理标记数组
    int *processed = (int *)calloc(input->ioVec.len, sizeof(int));
    if (processed == NULL) {
        free(output->sequence);
        return RETURN_ERROR;
    }

    HeadInfo currentHead = input->headInfo;
    for(uint32_t i = 0; i < output->len;i++){
        uint32_t minSeekTime = MAX_LPOS;
        uint32_t minIndex = 0;

        //寻找时间最短的IO请求
        for(uint32_t j = 0; j < input->ioVec.len; j++){
            if(processed[j] == 0){
                HeadInfo temp;
                temp.wrap = input->ioVec.ioArray[j].wrap;
                temp.lpos = input->ioVec.ioArray[j].startLpos;
                temp.status = HEAD_RW;
                uint32_t seekTime = SeekTimeCalculate(&currentHead, &temp);
                if(seekTime < minSeekTime){
                    minSeekTime = seekTime;
                    minIndex = j;
                }
            }
        }
        output->sequence[i] = input->ioVec.ioArray[minIndex].id;
        currentHead.wrap = input->ioVec.ioArray[minIndex].wrap;
        currentHead.lpos = input->ioVec.ioArray[minIndex].endLpos;
        currentHead.status = HEAD_RW;
        processed[minIndex] = 1;
    }

    return RETURN_OK;
}

int32_t GreedyPlus(const InputParam *input, OutputParam *output)
{
    int32_t ret;
    output->len = input->ioVec.len;
    output->sequence = (uint32_t *)malloc(output->len * sizeof(uint32_t));
    if (output->sequence == NULL) {
        return RETURN_ERROR;
    }

    // 初始化已处理标记数组
    int *processed = (int *)calloc(input->ioVec.len, sizeof(int));
    if (processed == NULL) {
        free(output->sequence);
        return RETURN_ERROR;
    }

    HeadInfo currentHead = input->headInfo;
    for(uint32_t i = 0; i < output->len;i++){
        uint32_t minSeekTime = MAX_LPOS;
        uint32_t minIndex = 0;

        //寻找时间最短的IO请求
        for(uint32_t j = 0; j < input->ioVec.len; j++){
            if(processed[j] == 0){
                HeadInfo temp;
                temp.wrap = input->ioVec.ioArray[j].wrap;
                temp.lpos = input->ioVec.ioArray[j].startLpos;
                temp.status = HEAD_RW;
                uint32_t seekTime = SeekTimeCalculate(&currentHead, &temp);
                uint32_t IOS = abs(input->ioVec.ioArray[j].endLpos - input->ioVec.ioArray[j].startLpos);
                double ans =  IOS*1.0/(IOS+seekTime)-abs(input->ioVec.ioArray[j].endLpos - currentHead.lpos)*1.0/IOS;
                if(ans < minSeekTime){
                    minSeekTime = ans;
                    minIndex = j;
                }
            }
        }
        output->sequence[i] = input->ioVec.ioArray[minIndex].id;
        currentHead.wrap = input->ioVec.ioArray[minIndex].wrap;
        currentHead.lpos = input->ioVec.ioArray[minIndex].endLpos;
        currentHead.status = HEAD_RW;
        processed[minIndex] = 1;
    }

    return RETURN_OK;
}

int32_t FIFO(const InputParam *input, OutputParam *output){
    int32_t ret;
     /* 算法示例：先入先出算法 */
    output->len = input->ioVec.len;
    for (uint32_t i = 0; i < output->len; i++) {
        output->sequence[i] = input->ioVec.ioArray[i].id;
    }
    return RETURN_OK;
}

/**
 * @brief  算法运行的主入口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return uint32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */
int32_t AlgorithmRun(const InputParam *input, OutputParam *output)
{
    int32_t ret;

    ret = Greedy(input, output);

    return RETURN_OK;
}
