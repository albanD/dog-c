#include "agregator.h"
#include "dogstatsd.h"


void addMetric(
	metricHash *metrics,
	char type,
	char *name,
	char *value,
	char *tags,
	char *sampleRate)
{
	int metricHashValue = hashMetric(name);

	metricChain *metricChainValue;

	metricChainValue = metrics->metricTable[metricHashValue];

	if(metricChainValue == NULL)
	{
		metrics->metricTable[metricHashValue] = newMetricChainElement(name, type, value, tags, sampleRate);
	}
	else
	{
		metricChain *selectedMetricElement;

		if( (selectedMetricElement = getMetricChainElementByName(metricChainValue, name)) )
		{
			addMetricData(selectedMetricElement, type, value, tags, sampleRate);
		}
		else
		{
			addMetricChainElement(metricChainValue, name, type, value, tags, sampleRate);
		}
	}

}


// Tags table management
void addMetricData(
	metricChain *metricChainElement,
	char type,
	char *value,
	char *tags,
	char *sampleRate)
{
	char **processedTags=NULL;
	int numberOfTags=0, tagsHashValue;

	tagsChain *tagsChainValue;
	processTags(tags, &processedTags, &numberOfTags);
	free(tags);

	tagsHashValue = hashTags(processedTags, numberOfTags);
	tagsChainValue  = metricChainElement->tagsTable[tagsHashValue];

	if(tagsChainValue == NULL)
	{
		metricChainElement->tagsTable[tagsHashValue] = newTagsChainElement(type, value, processedTags, numberOfTags, sampleRate);
	}
	else
	{
		tagsChain *selectedTagsElement;

		if( (selectedTagsElement = getTagsElementByTags(tagsChainValue, processedTags, numberOfTags)) )
		{
			addTagsData(selectedTagsElement, value, processedTags, numberOfTags, sampleRate);
		}
		else
		{
			addTagsChainElement(tagsChainValue, type, value, processedTags, numberOfTags, sampleRate);
		}
	}
}

void addTagsData(
	tagsChain *chainElement,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	switch (chainElement->metricType)
	{
		case 'g':
			updateGaugeData(&(chainElement->data->gaugeData), value, processedTags, numberOfTags, sampleRate);
			break;

		case 'c':
			updateCounterData(&(chainElement->data->counterData), value, processedTags, numberOfTags, sampleRate);
			break;

		case 'h':
			updateHistogramData(&(chainElement->data->histogramData), value, processedTags, numberOfTags, sampleRate);
			break;
	}
}


tagsChain* newTagsChainElement(
	char type,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	tagsChain *newElement;

	newElement = newTagsChain(type);

	addTagsData(newElement, value, processedTags, numberOfTags, sampleRate);

	return newElement;
}

void addTagsChainElement(
	tagsChain *tagsChainValue,
	char type,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	tagsChain *newElement;

	newElement = newTagsChainElement(type, value, processedTags, numberOfTags, sampleRate);

	newElement->next = tagsChainValue->next;
	tagsChainValue->next = newElement;
}

tagsChain *getTagsElementByTags(
	tagsChain *tagsChainValue,
	char **tags,
	int numberOfTags)
{
	tagsChain *current = tagsChainValue;
	char *oneStringTags = getOneStringTags(tags, numberOfTags);

	do {
		if(strcmp(getOneStringTagsFromMetricTagsChain(current), oneStringTags) == 0)
		{
			return current;
		}
	} while( (current = current->next) );

	return NULL;
}

tagsChain* newTagsChain(char type)
{
	tagsChain *newElement;
	newElement = malloc(sizeof(tagsChain));

	newElement->metricType = type;

	newElement->data = malloc(sizeof(metricData));

	newElement->next = NULL;

	return newElement;
}


// Tags formatting
void processTags(
	char *tags,
	char ***processedTags,
	int *numberOfTags)
{
	splitTags(tags, processedTags, numberOfTags);

	qsort(*processedTags, *numberOfTags, sizeof(char *), cstring_cmp);
}

int cstring_cmp(const void *a, const void *b)
    {
        const char **ia = (const char **)a;
        const char **ib = (const char **)b;
        return strcasecmp(*ia, *ib);
    }

void splitTags(
	char *tags,
	char ***processedTagsAdress,
	int *numberOfTags)
{
	char **temp=NULL, **processedTags=*processedTagsAdress;
	char *tag=NULL;
	int nextTagPosition = 0, initialStringSize = strlen(tags), i;

	tag = malloc(sizeof(char)*initialStringSize);
	
	do {
		nextTagPosition += getUntil(tags+nextTagPosition, ',', tag);

		temp = malloc(sizeof(char *)*(*numberOfTags+1));
		for(i=0; i<*numberOfTags; i++)
		{
			temp[i] = processedTags[i];
		}
		temp[*numberOfTags] = tag;

		if(processedTags)
			free(processedTags);
		processedTags = temp;
		temp = NULL;
		*numberOfTags = *numberOfTags + 1;

	} while(nextTagPosition < initialStringSize);
	*processedTagsAdress = processedTags;
}

char *getOneStringTags(
	char ** tags,
	int numberOfTags)
{
	int size = 0, position = 0, currentSize = 0, i;
	char *completeStr;

	for(i=0; i<numberOfTags; i++)
	{
		size += strlen(tags[i]);
	}

	completeStr = malloc(sizeof(char)*size);

	for(i=0; i<numberOfTags; i++)
	{
		currentSize = strlen(tags[i]);
		strcpy(&completeStr[position], tags[i]);
		position += currentSize;
	}

	return completeStr;
}

char *getOneStringTagsFromMetricTagsChain(
	tagsChain * tagsChainElement)
{
	switch (tagsChainElement->metricType)
	{
		case 'g':
			return getOneStringTags(
				tagsChainElement->data->gaugeData->tags,
				tagsChainElement->data->gaugeData->numberOfTags);
			break;

		case 'c':
			return getOneStringTags(
				tagsChainElement->data->counterData->tags,
				tagsChainElement->data->counterData->numberOfTags);
			break;

		case 'h':
			return getOneStringTags(
				tagsChainElement->data->histogramData->tags,
				tagsChainElement->data->histogramData->numberOfTags);
			break;
	}

	return NULL;
}


// Metric chain management
metricChain *getMetricChainElementByName(
	metricChain * metricChainValue,
	char *name)
{
	metricChain * current = metricChainValue;

	do {
		if(strcmp(current->name, name) == 0)
		{
			return current;
		}
	} while( (current = current->next) );

	return NULL;
}

void addMetricChainElement(
	metricChain * head,
	char *name,
	char type,
	char *value,
	char *tags,
	char *sampleRate)
{
	metricChain * newElement;

	newElement = newMetricChainElement(name, type, value, tags, sampleRate);

	newElement->next = head->next;
	head->next = newElement;
}

metricChain* newMetricChainElement(
	char *name,
	char type,
	char *value,
	char *tags,
	char *sampleRate)
{
	metricChain *newElement;

	newElement = newMetricChain(name);

	addMetricData(newElement, type, value, tags, sampleRate);

	return newElement;
}



// Hash table creation
metricChain* newMetricChain(char *name)
{
	int i;
	metricChain *newChain;
	newChain = malloc(sizeof(metricChain));

	newChain->name = name;

	newChain->tagsTable = malloc(sizeof(tagsChain *)*TAGS_HASH_TABLE_SIZE);
	newChain->size = TAGS_HASH_TABLE_SIZE;

	newChain->next = NULL;

	for(i=0; i<TAGS_HASH_TABLE_SIZE; i++)
	{
		newChain->tagsTable[i] = NULL;
	}

	return newChain;
}

metricHash* newMetricTable()
{
	int i;
	metricHash *newTable;
	newTable = malloc(sizeof(metricHash));

	newTable->metricTable = malloc(sizeof(metricChain *)*METRIC_HASH_TABLE_SIZE);
	newTable->size = METRIC_HASH_TABLE_SIZE;

	for(i=0; i<METRIC_HASH_TABLE_SIZE; i++)
	{
		newTable->metricTable[i] = NULL;
	}

	return newTable;
}



// Hash Functions

int hashMetric(char *str)
{
	return hash((unsigned char*) str, METRIC_HASH_TABLE_SIZE);
}

int hashTags(char **str, int nbr)
{
	int result;
	char *completeStr;

	completeStr = getOneStringTags(str, nbr);

	result = hash((unsigned char*)completeStr, TAGS_HASH_TABLE_SIZE);

	free(completeStr);

	return result;
}

int hash(unsigned char *str, int size)
{
    unsigned long hash = 5381;
    int c;

    while ( (c = *str++) )
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int) (hash%size);
}


// Metric update Functions

void updateGaugeData(
	gauge **gaugeDataOriginal,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	gauge *gaugeData = *gaugeDataOriginal;
	if(gaugeData == NULL)
	{
		gaugeData = malloc(sizeof(gauge));
		*gaugeDataOriginal = gaugeData;
	}
	double numericalValue = atof(value);
	gaugeData->value.floatValue = numericalValue;
	gaugeData->valueType = 'f';

	gaugeData->numberOfTags = numberOfTags;
	gaugeData->tags = processedTags;
}

void updateHistogramData(
	histogram **histogramDataOriginal,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	histogram *histogramData=*histogramDataOriginal;
	if(histogramData == NULL)
	{
		histogramData = malloc(sizeof(histogram));
		*histogramDataOriginal = histogramData;
	}
	valueType *tempPointer=NULL;
	double numericalValue = atof(value);

	tempPointer = malloc(sizeof(double)*(histogramData->numberOfSamples+1));

	memcpy(tempPointer, histogramData->samples, sizeof(valueType)*histogramData->numberOfSamples);
	memcpy(tempPointer+histogramData->numberOfSamples, &numericalValue, sizeof(valueType));

	if(histogramData->samples != NULL)
	{
		free(&histogramData->samples);
	}
	histogramData->samples = tempPointer;

	histogramData->sampleType = 'f';
	histogramData->numberOfSamples++;

	histogramData->numberOfTags = numberOfTags;
	histogramData->tags = processedTags;
}

void updateCounterData(
	counter **counterDataOriginal,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate)
{
	counter *counterData = *counterDataOriginal;
	if(counterData == NULL)
	{
		counterData = malloc(sizeof(counter));
		*counterDataOriginal = counterData;
	}
	counterData->valueType = 'f';
	counterData->value.floatValue = (atof(value)*(1/atof(sampleRate)));

	counterData->numberOfTags = numberOfTags;
	counterData->tags = processedTags;
}