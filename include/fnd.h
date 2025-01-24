#ifndef FND_H
#define FND_H

/*
    @brief
    FND 초기화 후 clear
*/
void fnd_init(void);

/*
    @brief
    FND 출력 값을 설정하고 출력 시작
    @param value 출력할 값 (0~9999)
*/
void fnd_set_print_value(int value);

/*
    @brief
    FND 출력을 취소하고 비움
*/
void fnd_clear(void);

#endif
