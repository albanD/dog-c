#ifndef _AGREGATOR_H
#define _AGREGATOR_H

#define METRIC_HASH_TABLE_SIZE 100
#define TAGS_HASH_TABLE_SIZE 50

#include <stdlib.h>
#include <stdio.h>

typedef union valueType
{
	int intValue;
	double floatValue;
} valueType;

typedef struct gauge
{
	char valueType;
	valueType value;

	int numberOfTags;
	char ** tags;
} gauge;

typedef struct histogram
{
	int count;

	char sampleType;
	int numberOfSamples;
	valueType *samples;

	int numberOfTags;
	char **tags;
} histogram;

typedef struct counter
{
	char valueType;
	valueType value;

	int numberOfTags;
	char **tags;
} counter;

typedef union metricData
{
	gauge *gaugeData;
	counter *counterData;
	histogram *histogramData;
} metricData;

typedef struct tagsChain
{
	char metricType;
	metricData *data;

	struct tagsChain *next;	
} tagsChain;

typedef struct metricChain
{
	char *name;

	tagsChain ** tagsTable;
	int size;

	struct metricChain *next;
} metricChain;

typedef struct metricHash
{
	metricChain ** metricTable;
	int size;

} metricHash;

void addMetric(
	metricHash *metrics,
	char type,
	char *name,
	char *value,
	char *tags,
	char *sampleRate);

void addMetricData(
	metricChain *metricChainElement,
	char type,
	char *value,
	char *tags,
	char *sampleRate);

void addTagsData(
	tagsChain *chainElement,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

tagsChain* newTagsChainElement(
	char type,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

void addTagsChainElement(
	tagsChain *tagsChainValue,
	char type,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

tagsChain *getTagsElementByTags(
	tagsChain *tagsChainValue,
	char **tags,
	int numberOfTags);

tagsChain* newTagsChain(char type);

void processTags(
	char *tags,
	char ***processedTags,
	int *numberOfTags);

int cstring_cmp(
	const void *a, 
	const void *b);

void splitTags(
	char *tags,
	char ***processedTagsAdress,
	int *numberOfTags);

char *getOneStringTags(
	char ** tags,
	int numberOfTags);

char *getOneStringTagsFromMetricTagsChain(
	tagsChain * tagsChainElement);

metricChain *getMetricChainElementByName(
	metricChain * metricChainValue,
	char *name);

void updateMetricChainElement(
	metricChain *selectedMetricElement,
	char type,
	char *value,
	char *tags,
	char *sampleRate);

void addMetricChainElement(
	metricChain * head,
	char *name,
	char type,
	char *value,
	char *tags,
	char *sampleRate);

metricChain* newMetricChainElement(
	char *name,
	char type,
	char *value,
	char *tags,
	char *sampleRate);

metricChain* newMetricChain(char *name);

metricHash* newMetricTable();

int hashMetric(char *str);

int hashTags(char **str, int nbr);

int hash(unsigned char *str, int size);

void updateGaugeData(
	gauge **gaugeData,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

void updateHistogramData(
	histogram **histogramData,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

void updateCounterData(
	counter **counterData,
	char *value,
	char **processedTags,
	int numberOfTags,
	char *sampleRate);

#endif