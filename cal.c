// ==========================================================================
// 계산기 연산식 처리 및 재귀 하향 파싱 엔진 구현부 (cal.c)
// ==========================================================================

#include "cal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static char expr[50];					// 수식 문자열 배열
static int16_t expr_idx = 0;			// 수식 배열 인덱스
static int16_t parse_idx = 0;			// 파싱 진행 인덱스
static uint8_t has_error = 0;			// 1: 연산 에러 발생 상태
static uint8_t should_reset = 0;		// 1: 다음 입력 시 리셋 필요한 상태

static int64_t evaluated_result = 0;	// 최종 연산 결과값
static uint8_t is_evaluated = 0;		// 1: 계산 완료되어 결과 출력할 상태

static int64_t parse_expr(void);
static int64_t parse_term(void);
static int64_t parse_factor(void);
static int64_t evaluate_expr(void);		// 수식 검사 및 파싱 시작 함수
static void int64_to_str(int64_t num, char *str);

// 계산기 초기화
void cal_init(void)
{
	cal_reset();
}

// 변수 및 버퍼 싹 비우기 (사이드 버튼 0번 연동)
void cal_reset(void)
{
	expr_idx = 0;
	expr[0] = '\0';
	has_error = 0;
	should_reset = 0;
	is_evaluated = 0;
	evaluated_result = 0;
}

// 한 글자 지우기 (사이드 버튼 1번 연동)
void cal_backspace(void)
{
	if (should_reset)
	{
		cal_reset();
		return;
	}
	if (expr_idx > 0)
	{
		expr_idx--;
		expr[expr_idx] = '\0';
	}
}

// 여는 괄호 '(' 입력 처리 (사이드 버튼 2번 연동)
void cal_open_paren(void)
{
	if (should_reset) cal_reset();
	if (expr_idx < (int16_t)sizeof(expr) - 1)
	{
		expr[expr_idx++] = '(';
		expr[expr_idx] = '\0';
	}
}

// 닫는 괄호 ')' 입력 처리 (사이드 버튼 3번 연동)
void cal_close_paren(void)
{
	if (should_reset) cal_reset();
	if (expr_idx < (int16_t)sizeof(expr) - 1)
	{
		expr[expr_idx++] = ')';
		expr[expr_idx] = '\0';
	}
}

// 키패드 문자 입력 처리 (숫자, 사칙연산자, 등호 분기)
void cal_input_key(uint8_t key)
{
	// 결과가 나온 상태에서 새 숫자를 누르면 화면 자동 초기화
	if (should_reset)
	{
		if ((key >= '0' && key <= '9') || key == '+' || key == '-' || key == '*' || key == '/')
		{
			cal_reset();
		}
	}

	// 숫자 또는 사칙연산 부호 기호 배열 적재
	if ((key >= '0' && key <= '9') || key == '+' || key == '-' || key == '*' || key == '/')
	{
		if (expr_idx < (int16_t)sizeof(expr) - 1)
		{
			expr[expr_idx++] = key;
			expr[expr_idx] = '\0';
		}
	}
	// '=' 입력 시 파서 작동 및 결과 산출 시동
	else if (key == '=')
	{
		expr[expr_idx] = '\0';
		evaluated_result = evaluate_expr();
		is_evaluated = 1;
		should_reset = 1;
	}
}

// LCD 1602 화면에 띄울 두 줄의 문자열 버퍼 가공 함수
void cal_get_display_strings(char *line1, char *line2)
{
	// 1번째 줄: 입력된 수식 (16자 넘어가면 좌측 자동 스크롤)
	if (expr_idx == 0)
	{
		sprintf(line1, "0               ");
	}
	else if (expr_idx <= 16)
	{
		sprintf(line1, "%-16s", expr);
	}
	else
	{
		sprintf(line1, "%-16s", expr + (expr_idx - 16));
	}

	// 2번째 줄: 결과값 또는 에러 메시지(=ERROR) 표출
	if (is_evaluated)
	{
		if (has_error)
		{
			sprintf(line2, "=ERROR          ");
		}
		else
		{
			char res_buf[25];
			char temp_buf[30];
			
			int64_to_str(evaluated_result, res_buf);
			sprintf(temp_buf, "=%s", res_buf);
			
			int16_t res_len = strlen(temp_buf);
			if (res_len <= 16)
			{
				sprintf(line2, "%-16s", temp_buf);
			}
			else
			{
				sprintf(line2, "%-16s", temp_buf + (res_len - 16));
			}
		}
	}
	else
	{
		sprintf(line2, "                ");
	}
}

// 파싱 1단계: 최우선순위 연산 (순수 숫자 결합 및 괄호 처리)
static int64_t parse_factor(void)
{
	int64_t value;
	
	// 괄호를 만나면 parse_expr을 재귀 호출하여 괄호 속 수식 먼저 격파
	if (expr[parse_idx] == '(')
	{
		parse_idx++;
		value = parse_expr();
		
		if (expr[parse_idx] == ')') parse_idx++;
		else has_error = 1; // 짝 괄호 탈출 실패 시 에러
	}
	else
	{
		value = 0;
		
		// 문자 기호로 들어온 연속된 숫자를 하나의 10진수 값으로 결합
		while (expr[parse_idx] >= '0' && expr[parse_idx] <= '9')
		{
			int64_t digit = expr[parse_idx] - '0';
			
			value = value * 10 + digit;
			parse_idx++;
		}
	}
	
	return value;
}

// 2단계: 중간 우선순위 연산 (곱셈, 나눗셈 처리)
static int64_t parse_term(void)
{
	int64_t value = parse_factor();
	
	while (expr[parse_idx] == '*' || expr[parse_idx] == '/')
	{
		char op = expr[parse_idx++];
		int64_t rhs = parse_factor();
		
		if (op == '*')
		{
			value = value * rhs;
		}
		else
		{
			value = (rhs != 0) ? value / rhs : 0; // 0으로 나누기 뻗음 예방
		}
	}
	
	return value;
}

// 3단계: 최하위 우선순위 연산 (덧셈, 뺄셈 처리)
static int64_t parse_expr(void)
{
	int64_t value = parse_term();
	
	while (expr[parse_idx] == '+' || expr[parse_idx] == '-')
	{
		char op = expr[parse_idx++];
		int64_t rhs = parse_term();
		
		if (op == '+')
		{
			value = value + rhs;
		}
		else
		{
			value = value - rhs;
		}
	}
	
	return value;
}

// 괄호 유효성 선행 검사 후 연산 시동
static int64_t evaluate_expr(void)
{
	int16_t paren_count = 0;
	
	// 연산 출발 전 괄호 개수 짝이 완벽한지 수식 전체 스캔
	for (int16_t i = 0; i < expr_idx; i++)
	{
		if (expr[i] == '(')       paren_count++;
		else if (expr[i] == ')')  paren_count--;
		
		if (paren_count < 0) break;
	}
	
	// 괄호 열고 안 닫았으면 연산 중단하고 즉시 에러 반환
	if (paren_count != 0)
	{
		has_error = 1;
		return 0;
	}
	
	parse_idx = 0;
	has_error = 0;
	
	return parse_expr(); // 최하위 레벨(덧셈/뺄셈) 시작
}

static void int64_to_str(int64_t num, char *str)
{
	char temp[25];
	int16_t i = 0, j = 0;
	uint8_t is_negative = 0;

	// 입력값이 0인 경우 예외 처리 후 즉시 종료
	if (num == 0)
	{
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	// 음수 처리: 마이너스 부호 플래그를 켜고 양수로 반전
	if (num < 0)
	{
		is_negative = 1;
		num = -num;
	}

	// 숫자를 10으로 나누며 뒷자리부터 한 글자씩 temp 배열에 거꾸로 적재
	while (num > 0)
	{
		temp[i++] = (num % 10) + '0'; // 숫자를 아스키(ASCII) 문자로 변환
		num /= 10;
	}

	// 음수였던 숫자는 최종 문자열(str) 맨 앞에 마이너스 부호 먼저 주입
	if (is_negative)
	{
		str[j++] = '-';
	}

	// temp 배열에 거꾸로 뒤집혀 담긴 문자들을 순서대로 꺼내어 str에 정방향 복사
	while (i > 0)
	{
		str[j++] = temp[--i];
	}
	
	str[j] = '\0'; // NULL(\0) 문자 마감 처리
}