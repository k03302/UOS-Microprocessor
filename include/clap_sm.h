#ifndef CLAP_SM_H
#define CLAP_SM_H

/*
    clap state machine는 다음과 2회의 연속된 박수를 인식한다.
    아래와 같은 소리 패턴을 2회의 연속된 박수라고 간주한다.
    박수를 인식한 후 finished 상태가 된다.
    박수를 인식하는 세부 조건은 다음과 같다.

         /\            /\
        /  \          /  \
       /    \        /    \
------/      \------/      \------

    1. 박수소리가 너무 오래 지속되면 일반 소음으로 간주하여 초기화한다.
    2. 박수소리가 너무 짧게 지속되면 지속시간이 확보될 때까지 무시한다.
    3. 두 번째 박수가 너무 늦게 시작하면 첫 번째 박수로 간주한다.
*/

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