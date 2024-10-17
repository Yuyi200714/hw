#include <stdio.h>
#include <stdlib.h>
#include "algorithm.h"
#include <limits.h>
#include <math.h>
/**
 * @brief  算法接口
 * @param  input            输入参数
 * @param  output           输出参数
 * @return int32_t          返回成功或者失败，RETURN_OK 或 RETURN_ERROR
 */

int compareIO(const void *a, const void *b) {
    IOUint *ioA = (IOUint *)a;
    IOUint *ioB = (IOUint *)b;
    return (ioA->startLpos - ioB->startLpos);
}

int calculatePriority(const IOUint *io1, const IOUint *io2) {
    if ((io1->wrap % 2 == 1 && io2->wrap % 2 == 1 && io1->endLpos > io2->startLpos) ||
        (io1->wrap % 2 == 0 && io2->wrap % 2 == 0 && io1->endLpos < io2->startLpos)) {
        return 1;
    } else if ((io1->wrap % 2 == 1 && io2->wrap % 2 == 1 && io1->endLpos <= io2->startLpos) ||
               (io1->wrap % 2 == 0 && io2->wrap % 2 == 0 && io1->endLpos >= io2->startLpos)) {
        return 3;
    } else {
        return 2;
    }
}

int32_t BlcokPriority(const InputParam *input, OutputParam *output)
{
    int32_t ret;
    if (input == NULL || output == NULL) {
        return RETURN_ERROR;
    }


    // 创建一个新的数组来存储排序后的 IO 请求
    IOUint *sortedIOs = (IOUint *)malloc(input->ioVec.len * sizeof(IOUint));
    if (sortedIOs == NULL) {
        free(output->sequence);
        return RETURN_ERROR;
    }
    for (uint32_t i = 0; i < input->ioVec.len; i++) {
        sortedIOs[i] = input->ioVec.ioArray[i];
    }

    // 按照 startLpos 对 IO 请求进行排序
    qsort(sortedIOs, input->ioVec.len, sizeof(IOUint), compareIO);

    // 计算每个块的大小
    uint32_t blockSize = (uint32_t)(input->ioVec.len/5);
    uint32_t blockNum = (input->ioVec.len + blockSize - 1) / blockSize;

    uint32_t index = 0;
    for (uint32_t b = 0; b < blockNum; b++) {
        uint32_t start = b * blockSize;
        uint32_t end = (b + 1) * blockSize;
        if (end > input->ioVec.len) {
            end = input->ioVec.len;
        }
        // 输出当前块内所有 IO 的 ID
        printf("Block %u: ", b);
        for (uint32_t i = start; i < end; i++) {
            printf("%u ", sortedIOs[i].id);
        }
        printf("\n");

        // 标记已处理的 IO
        int *processed = (int *)calloc(end - start, sizeof(int));
        while(1){
            // 计算每个块内的 IO 优先级数量
            int **priorityCount = (int **)malloc((end - start) * sizeof(int *));
            for (uint32_t i = 0; i < end - start; i++) {
                if(processed[i] == 1){
                    continue;
                }
                priorityCount[i] = (int *)calloc(3, sizeof(int));
                for (uint32_t j = 0; j < end - start; j++) {
                    if (i != j && !processed[j]) {
                        int priority = calculatePriority(&sortedIOs[start + i], &sortedIOs[start + j]);
                        priorityCount[i][priority - 1]++;
                    }
                }
            }

            //输出块内每个 IO 的 ID 及其三个优先级的数量
            for (uint32_t i = 0; i < end - start; i++) {
                if (processed[i] == 1)
                    continue;
                printf("IO %u: First Priority: %d, Second Priority: %d, Third Priority: %d, wrap = %u, startLpos = %u, endLpos = %u\n",
                    sortedIOs[start + i].id, priorityCount[i][0], priorityCount[i][1], priorityCount[i][2],sortedIOs[start + i].wrap,sortedIOs[start + i].startLpos,sortedIOs[start + i].endLpos);
            }



            // 选择拥有最多第一优先级的 IO
            int maxFirstPriority = -1;
            int selectedIndex = -1;
            int flag = -1;
            for (uint32_t i = 0; i < end - start; i++) {
                if (!processed[i] && priorityCount[i][0] > maxFirstPriority) {
                    maxFirstPriority = priorityCount[i][0];
                    selectedIndex = i;
                    flag=1;
                }
            }
            if (selectedIndex == -1) {
                // 如果没有找到拥有最多第一优先级的 IO，就选择拥有最多第二优先级的 IO
                int maxSecondPriority = -1;
                for (uint32_t i = 0; i < end - start; i++) {
                    if (!processed[i] && priorityCount[i][1] > maxSecondPriority) {
                        maxSecondPriority = priorityCount[i][1];
                        selectedIndex = i;
                        flag=2;
                    }
                }   
            }
            if (selectedIndex == -1) {
                // 如果没有找到拥有最多第二优先级的 IO，就选择拥有最多第三优先级的 IO
                int maxThirdPriority = -1;
                for (uint32_t i = 0; i < end - start; i++) {
                    if (!processed[i] && priorityCount[i][2] > maxThirdPriority) {
                        maxThirdPriority = priorityCount[i][2];
                        selectedIndex = i;
                        flag=3;
                    }
                }
            }
            if (selectedIndex == -1) {
                break;
            }
            
            processed[selectedIndex] = 1;
            output->sequence[index++] = sortedIOs[start + selectedIndex].id;
            printf("选中的io为 %u 优先级链为 %d\n", sortedIOs[start + selectedIndex].id, flag);

            // 将选中的 IO 和它的第flag优先级 IO 按照 startLpos 的顺序放入 output 中
            int wrap = sortedIOs[start + selectedIndex].wrap;
            int last = -1;
            if (wrap % 2 == 0) {
                // wrap 为偶数，按照 startLpos 递增的顺序放入 output 中
                for (uint32_t i = 0; i < end - start; i++) {
                    if (!processed[i] && calculatePriority(&sortedIOs[start + selectedIndex], &sortedIOs[start + i]) == flag) {
                        if(last == -1){
                            processed[i] = 1;
                            output->sequence[index++] = sortedIOs[start + i].id;
                            last = i;
                        }
                        else if(calculatePriority(&sortedIOs[start + last], &sortedIOs[start + i]) == flag){
                            processed[i] = 1;
                            output->sequence[index++] = sortedIOs[start + i].id;
                            last = i;
                        }
                    }
                }
            } else {
                // wrap 为奇数，按照 startLpos 递减的顺序放入 output 中
                for (int32_t i = end - start - 1; i >= 0; i--) {
                    if (!processed[i] && calculatePriority(&sortedIOs[start + selectedIndex], &sortedIOs[start + i]) == flag) {
                        if(last == -1){
                            processed[i] = 1;
                            output->sequence[index++] = sortedIOs[start + i].id;
                            last = i;
                        }
                        else if(calculatePriority(&sortedIOs[start + last], &sortedIOs[start + i]) == flag){
                            processed[i] = 1;
                            output->sequence[index++] = sortedIOs[start + i].id;
                            last = i;
                        }
                    }
                }
            }
            if(index == end){
                //输出当前块内的 IO ID 按顺序
                printf("当前块内的 IO ID 顺序为: ");
                for (uint32_t i = start; i < end; i++) {
                    printf("%u ", output->sequence[i]);
                }
                printf("\n");
                break;
            }
        }        
    }

    free(sortedIOs);
    return RETURN_OK;
}

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

    // 初始化已处理标记数组
    int *processed = (int *)calloc(input->ioVec.len, sizeof(int));
    if (processed == NULL) {
        free(output->sequence);
        return RETURN_ERROR;
    }

    HeadInfo currentHead = input->headInfo;
    for(uint32_t i = 0; i < output->len;i++){
        double maxSeekTime;
        uint32_t minIndex = 0;
        bool flag=0;
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
                //printf("seekTime: %u, IOS: %u, ans: %f\n", seekTime, IOS, ans);
                if(flag==0){
                    maxSeekTime = ans;
                    minIndex = j;
                    flag=1;
                }
                if(ans > maxSeekTime){
                    maxSeekTime = ans;
                    minIndex = j;
                }
            }
        }
        //printf("选中的io为 %u 值为 %f\n", input->ioVec.ioArray[minIndex].id, maxSeekTime);
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
    // printf("Greedy:\n");
    ret = Greedy(input, output);
    // for(uint32_t i = 0; i < output->len; i++){
    //     printf("%u ", output->sequence[i]);
    // }
    // printf("\n");

    return RETURN_OK;
}

int32_t AlgorithmRun2(const InputParam *input, OutputParam *output)
{
    int32_t ret;
    // printf("BlcokPriority:\n");
    ret = BlcokPriority(input, output);
    // for(uint32_t i = 0; i < output->len; i++){
    //     printf("%u ", output->sequence[i]);
    // }
    // printf("\n");

    return RETURN_OK;
}
