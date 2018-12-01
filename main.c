#include <stdio.h>
#include <string.h>
#include <stdlib.h>  

typedef enum {false, true} bool;
#define maxLineSize 255

size_t countCharOccurences(char *s, char c)
{
	size_t count = 0;
	for (size_t i = 0; s[i]; ++i) count += (s[i] == c);
	return count;
}

void sliceStr(const char *s, char * token, size_t start, size_t end)
{
	size_t j = 0;
	for (size_t i = start; i <= end; ++i)
		token[j++] = s[i];
	token[j] = 0;	// manual null termination
}

size_t countlines(const char *filename)
{
	// count the number of lines in the file                                    
	FILE *file = fopen(filename, "r");
	int ch = 0;
	size_t lines = 0;

	if (file)
	{
		lines++;
		while ((ch = fgetc(file)) != EOF)
		{
			if (ch == '\n')
				lines++;
		}
		fclose(file);
	}
	return lines;
}

size_t strlstchar(const char *s, const char c)
{
	char *cptr = strrchr(s, c);
	return cptr - s;
}

int main(int argc, char **argv)
{
	// The file 'lp1.txt' should be located in the same folder as the executable
	char *path = NULL;
	size_t pathSize = strlstchar(argv[0], '\\');
	path = (char *)malloc((pathSize) * sizeof(char));
	sliceStr(argv[0], path, 0, pathSize - 1);

	char *inFilename = (char *)malloc((strlen(path) + 8) * sizeof(char));
	char *outFilename = (char *)malloc((strlen(path) + 8) * sizeof(char));

	strncpy(inFilename, path, (strlen(path) + 8));
	strcat(inFilename, "\\lp1.txt");

	strncpy(outFilename, path, (strlen(path) + 8));
	strcat(outFilename, "\\lp2.txt");

	free(path);

	size_t numOfArows = countlines(inFilename) - 1;

	// Initialize A matrix
	double **A;
	A = (double **)malloc(numOfArows * sizeof(double *));
	size_t Arow_id = 0;
	size_t numOfAcols = 0;

	// Initialize b vector
	double *b = NULL;
	b = (double *)malloc(numOfArows * sizeof(double));
	size_t b_id = 0;

	// Initialize Eqin vector
	int *Eqin = NULL;
	Eqin = (int *)malloc(numOfArows * sizeof(int));
	size_t Eqin_id = 0;

	// objective function's coefficients
	double *c = NULL;

	// Optimization problem type
	int MinMax = 0;

	// subject to
	char *st = NULL;

	// delimiters
	char delimiter1 = ' ';
	char delimiter2 = '+';

	FILE * file;

	file = fopen(inFilename, "r");
	free(inFilename);
	bool firstLine = true;
	bool secondLine = true;
	
	char *line = NULL;
	line = (char *)malloc(maxLineSize * sizeof(char));
	if (file)
	{	
		while (fgets(line, maxLineSize, (FILE*)file))
		{
			if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
			// Split the objective function line based on the ' ' delimiter
			char* token;
			char** tokens;
			size_t numOfTokens = countCharOccurences(line, delimiter1) + 1;
			tokens = (char **)malloc(numOfTokens * sizeof(char*));

			size_t len = strlen(line);
			size_t token_id, start, end;
			token_id = start = end = 0;
			for (size_t i = 0; i < len; ++i)
			{
				if (line[i] == delimiter1)
				{
					end = i;
					token = (char *)malloc((end - start + 1) * sizeof(char));
					sliceStr(line, token, start, end - 1);
					tokens[token_id] = token;
					token_id++;
					start = end + 1;
				}
			}
			// Read the last slice of the line from the last occurence of ' '
			// up to the end of line
			end = len;
			token = (char *)malloc((end - start + 1) * sizeof(char));
			sliceStr(line, token, start, end - 1);
			tokens[token_id] = token;

			if (firstLine)
			{
				firstLine = false;
				
				// Get the number of c coefficients by counting the number of '+' characters
				// number of c coefficients = (number of '+' characters) + 1
				size_t numOfCoeffs = countCharOccurences(line, delimiter2) + 1;
				c = malloc(numOfCoeffs * sizeof(double));
				
				// Set the type of the optimization problem
				MinMax = (strcmp("min", tokens[0]) == 0) ? -1 : 1;

				// Convert the objective function's coefficients
				// from char * to double
				size_t coeff_id = 0;
				for (size_t i = 1; i < numOfTokens; ++i)
				{
					if (strlen(tokens[i]) > 1)
					{
						char *rest;
						double coeff_d = strtod(tokens[i], &rest);
						c[coeff_id] = coeff_d;
						coeff_id++;
					}
				}

				// Clear memory
				//for (size_t i = 0; i < numOfTokens; i++)
				//	free(tokens[i]);
				free(tokens);
				continue;
			}
			else
			{
				size_t startOfTokens = -1;
				if (secondLine)
				{
					secondLine = false;
					// Parse the type of optimization problem
					st = (char *)malloc((strlen(tokens[0]) + 1) * sizeof(char));
					strcpy(st, tokens[0]);
					startOfTokens = 1;
				}
				else
				{
					startOfTokens = 0;
				}
				// Parse the coefficients of the A matrix
				size_t coeff_id = 0;
				double *Arow = NULL;
				size_t numOfCoeffs = countCharOccurences(line, delimiter2) + 1;
				numOfAcols = numOfCoeffs;
				Arow = (double *)malloc(numOfCoeffs * sizeof(double));
				for (size_t i = startOfTokens; i < (numOfTokens - 2); ++i)
				{
					if (strlen(tokens[i]) > 1)	// if token is not '+', '=', '>=' or '<='
					{
						char *rest = NULL;
						double coeff_d = strtod(tokens[i], &rest);
						Arow[coeff_id] = coeff_d;
						coeff_id++;
					}
				}
				A[Arow_id] = Arow;
				Arow_id++;

				// Parse '=', '>=' or '<=' symbol
				size_t symbol;
				if (strcmp(tokens[numOfTokens - 2], "=") == 0)
				{
					symbol = 0;
				}
				else if (strcmp(tokens[numOfTokens - 2], ">=") == 0)
				{
					symbol = 1;
				}
				else if (strcmp(tokens[numOfTokens - 2], "<=") == 0)
				{
					symbol = -1;
				}

				Eqin[Eqin_id] = symbol;
				Eqin_id++;

				// Parse the coefficients of b vector
				char *rest;
				double coeff_d = strtod(tokens[numOfTokens - 1], &rest);
				b[b_id] = coeff_d;
				b_id++;
			}
		}
		fclose(file);
	}

	// Save results to file
	file = fopen(outFilename, "w");
	free(outFilename);
	if (MinMax = -1)
	{
		fprintf(file, "min ");
	}
	else
	{
		fprintf(file, "max ");
	}
	fprintf(file, "c'x\n");
	fprintf(file, "%s ", st);
	fprintf(file, "Ax@b\n");
	fprintf(file, "x >= 0\n\n");
	fprintf(file, "where:\n\n");

	fprintf(file, "A = \n");
	for (size_t i = 0; i < numOfArows; ++i)
	{
		for (size_t j = 0; j < numOfAcols; j++)
		{
			if (j != (numOfAcols - 1))
				fprintf(file, "%.2f ", A[i][j]);
			else
				fprintf(file, "%.2f", A[i][j]);
		}
		fprintf(file, "\n");
	}

	fprintf(file, "\n");

	fprintf(file, "b = \n");
	for (size_t i = 0; i < numOfArows; ++i)
	{
		fprintf(file, "%.2f\n", b[i]);
	}

	fprintf(file, "\n");

	fprintf(file, "@ = \n");
	for (size_t i = 0; i < numOfArows; ++i)
	{
		fprintf(file, "%d\n", Eqin[i]);
	}

	fprintf(file, "\n");

	fprintf(file, "c = \n");
	for (size_t i = 0; i < numOfAcols; ++i)
	{
		fprintf(file, "%.2f\n", c[i]);
	}

	fclose(file);

	// Clear memory
	free(A);
	free(b);
	free(c);
	free(line);
	printf("\n");
	return 0;
}