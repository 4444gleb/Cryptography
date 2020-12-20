#define _CRT_SECURE_NO_WARNINGS
#define KEY_SIZE 16
#define BLOCK_SIZE 8

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h> 

uint64_t add_mod_65536(uint64_t a, uint64_t b);
uint16_t mul_mod_65537(uint64_t a, uint64_t b);
uint16_t modInverse(uint16_t a, uint32_t m);
uint16_t power(uint16_t x, uint16_t y, uint32_t m);
uint16_t gcd(uint16_t a, uint16_t b);
unsigned char *cyclic_left_shift_25(unsigned char * key);

void do_round(uint16_t *block, uint16_t *round_key);
unsigned char *cipher(unsigned char *block, unsigned char *key);

unsigned char *get_key();

int encrypt()
{
	//Получаем ключ
	unsigned char *key = get_key();
	if (key == -1)
	{
		return -1;
	}

	//Открываем файл, который надо зашифровать
	printf("%s\n", "Укажите файл для зашифрования (inputfile.txt): ");
	char inputfile[100];
	fseek(stdin, 0, SEEK_END);
	gets(inputfile);
	if (!strlen(inputfile))
	{
		strcpy(inputfile, "inputfile.txt");
	}
	else
	{
		int sl = strlen(inputfile) - 1;
		if ((inputfile[sl]) == '\n')
			inputfile[sl] = 0;
	}
	FILE *input_file;
	input_file = fopen(inputfile, "rb");
	if (input_file == NULL)
	{
		printf("Не удалось открыть файл для зашифрования");
		getchar();
		return -1;
	}


	//Открываем файл, в который будем писать зашифрованное
	printf("%s\n", "Укажите файл, куда поместить шифровку (outputfile.bin): ");
	char outputfile[100];
	fseek(stdin, 0, SEEK_END);
	gets(outputfile);
	if (!strlen(outputfile))
	{
		strcpy(outputfile, "outputfile.bin");
	}
	else
	{
		int sl = strlen(outputfile) - 1;
		if ((outputfile[sl]) == '\n')
			outputfile[sl] = 0;
	}
	FILE *output_file;
	output_file = fopen(outputfile, "wb");
	if (output_file == NULL)
	{
		printf("Не удалось открыть файл для зашифрования");
		getchar();
		return -1;
	}

	//Выделяем память на блок
	unsigned char *block = NULL;
	block = (unsigned char*)calloc(BLOCK_SIZE, sizeof(unsigned char));
	if (block == NULL)
	{
		return -1;
	}

	//Пока не конец файла, читаем блок и шифруем его, затем записываем его в выходной файл
	int i = 0;
	while (!feof(input_file))
	{
		i = fread(block, sizeof(unsigned char), BLOCK_SIZE, input_file);
		if (!i)
		{
			break;
		}
		if (i < BLOCK_SIZE)
		{
			for (i; i < BLOCK_SIZE; i++)
			{
				block[i] = '\0';
			}
		}
		block = cipher(block, key, 0);

		//Преобразование блока из двухбайтного в однобайтный вид
		unsigned char sym;
		for (int i = 0; i < 8; i += 2)
		{
			sym = block[i];
			block[i] = block[i + 1];
			block[i + 1] = sym;
		}

		//Записываем блок в файл
		fwrite(block, sizeof(unsigned char), BLOCK_SIZE, output_file);
	}
	fclose(input_file);
	fclose(output_file);
	return 0;
}



int decrypt()
{
	//Получаем ключ
	unsigned char *key = get_key();
	if (key == -1)
	{
		return -1;
	}

	//Открываем файл, который надо расшифровать
	printf("%s\n", "Укажите зашифрованный файл (outputfile.bin): ");
	char inputfile[100];
	fseek(stdin, 0, SEEK_END);
	gets(inputfile);
	if (!strlen(inputfile))
	{
		strcpy(inputfile, "outputfile.bin");
	}
	else
	{
		int sl = strlen(inputfile) - 1;
		if ((inputfile[sl]) == '\n')
			inputfile[sl] = 0;
	}
	FILE *input_file;
	input_file = fopen(inputfile, "rb");
	if (input_file == NULL)
	{
		printf("Не удалось открыть зашифрованный файл");
		getchar();
		return -1;
	}

	//Открываем файл, в который будем писать расшифрованное
	printf("%s\n", "Укажите файл, куда поместить расшифрованную информацию (decrypted.txt): ");
	char outputfile[100];
	fseek(stdin, 0, SEEK_END);
	gets(outputfile);
	if (!strlen(outputfile))
	{
		strcpy(outputfile, "decrypted.txt");
	}
	else
	{
		int sl = strlen(outputfile) - 1;
		if ((outputfile[sl]) == '\n')
			outputfile[sl] = 0;
	}
	FILE *output_file;
	output_file = fopen(outputfile, "wb");
	if (output_file == NULL)
	{
		printf("Не удалось открыть файл для расшифрования");
		getchar();
		return -1;
	}

	//Выделяем память на хранение блока входных данных
	unsigned char *block = NULL;
	block = (unsigned char*)calloc(BLOCK_SIZE, sizeof(unsigned char));
	if (block == NULL)
	{
		return -1;
	}

	//Пока не конец файла, читаем блок и расшифровываем его, затем записываем его в выходной файл
	int i = 0;
	while (!feof(input_file))
	{
		i = fread(block, sizeof(unsigned char), BLOCK_SIZE, input_file);
		if (!i)
		{
			break;
		}
		if (i < BLOCK_SIZE)
		{
			for (i; i < BLOCK_SIZE; i++)
			{
				block[i] = '\0';
			}
		}

		block = cipher(block, key, 1);

		//Преобразование блока из двухбайтного в однобайтный вид
		unsigned char sym;
		for (int i = 0; i < 8; i += 2)
		{
			sym = block[i];
			block[i] = block[i + 1];
			block[i + 1] = sym;
		}

		fwrite(block, sizeof(unsigned char), BLOCK_SIZE, output_file);
	}
	fclose(input_file);
	fclose(output_file);
	return 0;
}


int main()
{
	setlocale(LC_ALL, "");

	printf("%s\n\n", "Шифрование по алгоритму IDEA");
	printf("%s\n", "Выберите режим работы: нажмите e для зашифрования, d для дешифрования");

	char mode;
	mode = getchar();

	//Считываем режим работы
	while (!((mode == 'e') || (mode == 'E') || (mode == 'd') || (mode == 'D')))
	{
		printf("%s\n", "Пожалуйста, нажмите e для зашифрования, d для дешифрования, k для генерации ключа");
		/*scanf("%c", &mode);*/
		mode = getchar();
	}
	_flushall();
	if ((mode == 'e') || (mode == 'E'))
	{
		//Шифрование
		if (encrypt() == -1)
		{
			printf("%s\n", "Ошибка открытия файла!");
			getchar();
			return 0;
		}
		printf("%s\n", "Шифрование завершено");
	}
	else if ((mode == 'd') || (mode == 'd'))
	{
		//Дешифрование
		if (decrypt() == -1)
		{
			printf("%s\n", "Ошибка открытия файла!");
			getchar();
			return 0;
		}
		printf("%s\n", "Дешифрование завершено");
	}

	getchar();
	return 0;
}

uint16_t gcd(uint16_t a, uint16_t b)
{
	//Наибольший общий делитель
	if (a == 0)
		return b;
	return gcd(b % a, a);
}

uint16_t power(uint16_t x, uint16_t y, uint32_t m)
{
	//Вычисление x ^ y по модулю m
	if (y == 0)
		return 1;
	uint16_t p = power(x, y / 2, m) % m;
	p = (p * p) % m;
	return (y % 2 == 0) ? p : (x * p) % m;
}

uint16_t modInverse(uint16_t a, uint32_t m)
{
	//Нахождение обратного по модулю - мультипликативная инверсия
	uint16_t inverse = 0;
	int g = gcd(a, m);
	if (g != 1)
		inverse = NULL;
	else
	{
		// Если a и m относительно простые, то по модулю обратное
		// это режим ^ (m-2) m
		inverse = power(a, m - 2, m);
	}
}

unsigned char *cipher(unsigned char *block, unsigned char *key, int decipher)
{
	int rounds_count = 9;

	//Преобразование блока из байтового в двухбайтовый вид
	uint16_t *block2 = NULL;
	block2 = (uint16_t *)malloc(BLOCK_SIZE / 2 * sizeof(uint16_t));
	for (int i = 0; i < 8; i += 2)
	{
		block2[i / 2] = (block[i] << 8) | block[i + 1];
	}

	//Выделяем память под раундовые ключи
	uint16_t **round_keys = NULL;
	round_keys = (uint16_t **)malloc(rounds_count * sizeof(uint16_t *));
	for (int i = 0; i < (rounds_count - 1); i++)
	{
		round_keys[i] = (uint16_t *)malloc(6 * sizeof(uint16_t));
	}
	round_keys[rounds_count - 1] = (uint16_t *)malloc(4 * sizeof(uint16_t));

	//Процедура расширения ключа: получаем необходимые 52 ключа - по 6 на раунд и еще 4 для выходного преобразования
	int k = 0;
	uint16_t *key2 = NULL;
	key2 = (uint16_t *)malloc(KEY_SIZE / 2 * sizeof(uint16_t));

	while (k < ((rounds_count - 1) * 6 + 4))  //k < 52
	{
		if (!(k % 8))
		{
			//Циклический сдвиг на 25 бит
			if (k != 0)
				cyclic_left_shift_25(key);

			//Преобразование ключа из байтового в двухбайтовый вид
			for (int i = 0; i < 16; i += 2)
			{
				key2[i / 2] = (key[i] << 8) | key[i + 1];
			}
		}
		round_keys[k / 6][k % 6] = key2[k % 8];
		k++;
	}


	//Если дешифрация, то меняем порядок ключей и преобразовываем их с помощью мультипликативной и аддитивной инверсии согласно алгоритму
	if (decipher)
	{
uint16_t buf = 0;
k = 0;  //k = 51

uint16_t **round_keys_decipher = NULL;
round_keys_decipher = (uint16_t **)malloc(rounds_count * sizeof(uint16_t *));
for (int i = 0; i < (rounds_count - 1); i++)
{
	round_keys_decipher[i] = (uint16_t *)malloc(6 * sizeof(uint16_t));
}
round_keys_decipher[rounds_count - 1] = (uint16_t *)malloc(4 * sizeof(uint16_t));

for (int i = 0; i < 4; i++)
{
	if ((i == 0) || (i == 3))
		round_keys_decipher[0][i] = modInverse(round_keys[8][i], 65537);
	else
		round_keys_decipher[0][i] = (65536 - round_keys[8][i]) % 65536;
}

for (int i = 1; i < 9; i++)
{
	for (int j = 0; j < 4; j++)
	{
		if ((j == 0) || (j == 3))
			round_keys_decipher[i][j] = modInverse(round_keys[8 - i][j], 65537);
		else if (j == 1)
			round_keys_decipher[i][j] = (65536 - round_keys[8 - i][j + 1]) % 65536;
		else
			round_keys_decipher[i][j] = (65536 - round_keys[8 - i][j - 1]) % 65536;
	}
	round_keys_decipher[i - 1][4] = round_keys[7 - i + 1][4];
	round_keys_decipher[i - 1][5] = round_keys[7 - i + 1][5];
}
round_keys_decipher[8][1] = (65536 - round_keys[0][1]) % 65536;
round_keys_decipher[8][2] = (65536 - round_keys[0][2]) % 65536;

round_keys = round_keys_decipher;
	}


	//Выполняем 8 раундов преобразований
	for (int i = 0; i < (rounds_count - 1); i++)
	{
		do_round(block2, round_keys[i]);
	}

	//Выходное преобразование
	block2[0] = mul_mod_65537(block2[0], round_keys[8][0]);
	uint16_t D3 = add_mod_65536(block2[1], round_keys[8][2]);
	block2[1] = add_mod_65536(block2[2], round_keys[8][1]);
	block2[2] = D3;
	block2[3] = mul_mod_65537(block2[3], round_keys[8][3]);
	return block2;
}

void do_round(uint16_t *block, uint16_t *round_key)
{
	//Выполняем преобразования раунда
	uint16_t D1 = block[0];
	uint16_t D2 = block[1];
	uint16_t D3 = block[2];
	uint16_t D4 = block[3];

	uint16_t K1 = round_key[0];
	uint16_t K2 = round_key[1];
	uint16_t K3 = round_key[2];
	uint16_t K4 = round_key[3];
	uint16_t K5 = round_key[4];
	uint16_t K6 = round_key[5];

	uint16_t A = mul_mod_65537(D1, K1);
	uint16_t B = add_mod_65536(D2, K2);
	uint16_t C = add_mod_65536(D3, K3);
	uint16_t D = mul_mod_65537(D4, K4);
	uint16_t E = A ^ C;
	uint16_t F = B ^ D;

	D1 = A ^ mul_mod_65537(add_mod_65536(F, mul_mod_65537(E, K5)), K6);
	D2 = C ^ mul_mod_65537(add_mod_65536(F, mul_mod_65537(E, K5)), K6);
	D3 = B ^ add_mod_65536(mul_mod_65537(E, K5), mul_mod_65537(add_mod_65536(F, mul_mod_65537(E, K5)), K6));
	D4 = D ^ add_mod_65536(mul_mod_65537(E, K5), mul_mod_65537(add_mod_65536(F, mul_mod_65537(E, K5)), K6));

	block[0] = D1;
	block[1] = D2;
	block[2] = D3;
	block[3] = D4;
}

unsigned char * get_key()
{
	//Получаем ключ из файла
	printf("%s\n", "Укажите ключевой файл (key.txt): ");
	char keyfile[100];
	fseek(stdin, 0, SEEK_END);
	fgets(keyfile, 100, stdin);
	if ((!strlen(keyfile)) || (keyfile[0] == '\n'))
	{
		strcpy(keyfile, "key.txt");
	}
	else
	{
		int sl = strlen(keyfile) - 1;
		if ((keyfile[sl]) == '\n')
			keyfile[sl] = 0;
	}
		

	FILE *file;

	file = fopen(keyfile, "rb");
	if (file == NULL)
	{
		printf("Не удалось открыть файл");
		getchar();
		return -1;
	}

	unsigned char *key = NULL;
	key = (unsigned char*)calloc(KEY_SIZE, sizeof(unsigned char));
	if (key == NULL)
		return 0;

	int i = fread(key, 1, KEY_SIZE, file);
	if (i == 0)
	{
		printf("Файл пуст");
		getchar();
		return -1;
	}

	fclose(file);

	//Если ключ недостаточной длины, циклически его дополняем
	if (i < KEY_SIZE)
	{
		int j = i;
		while (i < KEY_SIZE)
		{
			key[i] = key[i % j];
			i++;
		}
	}
	return key;
}

uint16_t mul_mod_65537(uint64_t a, uint64_t b)
{
	int64_t p;
	uint64_t q;
	if (a == 0)
		p = 65537 - b;
	else
		if (b == 0)
			p = 65537 - a;
		else
		{
			q = a * b;
			p = (q & 65535) - (q >> 16);
			if (p <= 0)
			{
				p = -p;
				p %= 65537;
				p = -p;
				p += 65537;
			}
		}
	return (uint16_t)(p & 65535);
}

uint64_t add_mod_65536(uint64_t a, uint64_t b)
{
	return (((a % 65536) + (b % 65536)) % 65536);
}

unsigned char *cyclic_left_shift_25(unsigned char * key)
{
	int bit_shift = 25;

	uint64_t first64bit = 0;
	uint64_t second64bit = 0;
	for (int i = 0; i < KEY_SIZE / 2; i++)
	{
		first64bit = (first64bit << 8) | key[i];
		second64bit = (second64bit << 8) | key[i + 8];
	}

	uint64_t first25bit = first64bit >> (sizeof(uint64_t) * 8 - bit_shift);
	uint64_t second25bit = second64bit >> (sizeof(uint64_t) * 8 - bit_shift);

	first64bit = (first64bit << bit_shift) | second25bit;
	second64bit = (second64bit << bit_shift) | first25bit;

	unsigned char c = 0;
	unsigned char d = 0;
	for (int i = 0; i < KEY_SIZE / 2; i++)
	{
		key[i] = (first64bit << 8 * i) >> (sizeof(uint64_t) * 8 - 8);
		key[i + 8] = (second64bit << 8 * i) >> (sizeof(uint64_t) * 8 - 8);
	}

	return key;
}