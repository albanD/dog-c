#include "dogstatsd.h"
#include "agregator.h"


int main(int argc, char* argv[])
{
	startServer();
	
	return 0;
}

void startServer()
{
	int fd;
	struct sockaddr_in localAddr, remAddr;
	socklen_t addrlen = sizeof(remAddr);
	int recvlen;
	char buff[BUFFSIZE];
	unsigned int alen;

	metricHash *metrics=NULL;
	metrics = newMetricTable();

	char *metric[5], *line=NULL, *lastElement=NULL, *unknownMetric=NULL;

	memset((char *) &localAddr, 0, sizeof(localAddr));

	int nextLinePosition, nextEntryPosition, size;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Cannot create socket");
		return;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(PORT);

	if (bind(fd, (struct sockaddr*) &localAddr, sizeof(localAddr)) < 0)
	{
		perror("Fail to bind to port");
		return;
	}

	alen = sizeof(localAddr);
	if (getsockname(fd, (struct sockaddr *)&localAddr, &alen) < 0) {
		perror("getsockname failed");
		return;
	}

	printf("bind complete. Port number = %d\n", ntohs(localAddr.sin_port));

	printf("Listening...\n");

	for (;;)
	{
		recvlen = recvfrom(fd, buff, BUFFSIZE, 0, (struct sockaddr*) &remAddr, &addrlen);
		printf("\nReceived %d bytes\n", recvlen);

		if (recvlen > 0)
		{
			buff[recvlen] = 0;
			printf("Packet content: \"%s\"\n", buff);
			nextLinePosition = 0;
			char * res=NULL;
			res=realloc(res, sizeof(char)*(strlen(buff)));
			strncpy(res, buff, strlen(buff));
			line = realloc(line, strlen(res));
			while (nextLinePosition < recvlen-1 && getUntil((res + nextLinePosition), '\n', line))
			{
				printf("Processing line: \"%s\"\n", line);
				int lineSize = strlen(line);
				initMetric(metric, lineSize);
				unknownMetric = realloc(unknownMetric, sizeof(char)*lineSize);
				lastElement = realloc(lastElement, sizeof(char)*lineSize);
				nextLinePosition += lineSize;
				nextEntryPosition = getUntil(line, ':', metric[0]);
				nextEntryPosition += getUntil(line+nextEntryPosition, '|', metric[1]);
				nextEntryPosition += getUntil(line+nextEntryPosition, '\n', lastElement);

				size = strlen(lastElement);
				nextEntryPosition = getUntil(lastElement, '|', metric[2]);
				if (nextEntryPosition <= size)
				{
					nextEntryPosition += getUntil(lastElement+nextEntryPosition, '|', unknownMetric);
					setUnknowMetric(metric, unknownMetric);
					if (nextEntryPosition <= size)
					{
						nextEntryPosition += getUntil(lastElement+nextEntryPosition, '|', unknownMetric);
						setUnknowMetric(metric, unknownMetric);
						if (nextEntryPosition <= size)
						{
							perror("Too many \"|\" in the request");
						}
					}
				}

				printf("\"%s/%s/%s/%s/%s\"\n", metric[0], metric[1], metric[2], metric[3], metric[4]);
				addMetric(metrics, metric[2][0], metric[0], metric[1], metric[3], metric[4]);
			}
		}

		printf("Packet processed\n\n");
	}
}

int getUntil(char *string, char pattern, char* result)
{
	int begin = 0, size = 0, i = 0;
	//printf("pattern: \"%c\" -%s- \n", pattern, string);

	if (string[i] == 0)
	{
		return 0;
	}

	while(string[i] == pattern)
	{
		begin++;
		i++;
		if (string[i] == 0)
		{
			return 0;
		}
	}

	while(string[i] != pattern)
	{
		if (string[i] == 0)
		{
			break;
		}
		size++;
		i++;
	}

	strncpy(result, string+begin, size);
	result[size] = 0;

	//printf("result is \"%s\", %c, %s\n", result, pattern, string);
	return begin + size + 1;
}

void setUnknowMetric(char *metric[5], char *unknownMetric)
{
	printf("Setting: \"%s\"\n", unknownMetric);
	if (unknownMetric[0] == '@')
	{
		strncpy(metric[4], unknownMetric+1, strlen(unknownMetric));
	}
	else if (unknownMetric[0] == '#')
	{
		strncpy(metric[3], unknownMetric+1, strlen(unknownMetric));
	}
	return;
}

void initMetric(char *metric[5], int lineSize)
{
	metric[0] = malloc(sizeof(char)*lineSize);
	metric[1] = malloc(sizeof(char)*lineSize);
	metric[2] = malloc(sizeof(char)*lineSize);
	metric[3] = malloc(sizeof(char)*lineSize);
	metric[4] = malloc(sizeof(char)*lineSize);
}