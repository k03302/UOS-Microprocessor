#ifndef CLAP_SM_H
#define CLAP_SM_H

/*
    @brief
    2회 박수를 인식하면 finished 상태가 되는 박수 상태머신

    @note
    박수 상태머신은 현재 ADC가 sound(ADC2)를 이용한다고 가정함
*/
void clap_state_machine();

/*
    @brief
    박수 상태머신이 finished 상태인지 반환

    @return
    1 if finished, 0 otherwise
*/
int clap_state_machine_finished();

/*
    @brief
    박수 상태머신 초기화
*/
void clap_state_machine_initialize();

#endif