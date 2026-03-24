#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_MAX_CITIES 1000
#define INFINITY_DISTANCE 1000000000
#define INVALID_STATE (-1)
#define NO_CITY_FOUND (-1)

typedef struct {
    int* citiesArray;
    int citiesCount;
    int citiesCapacity;
} StateCities;

typedef struct {
    int cityFrom;
    int cityTo;
    int roadLength;
} Road;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Использование: ./program <input.txt>");
        return 1;
    }

    char* inputFilePath = argv[1];

    FILE* inputFilePointer = fopen(inputFilePath, "r");
    if (inputFilePointer == NULL) {
        printf("Ошибка: не удалось открыть файл \"%s\"\n", inputFilePath);
        return 1;
    }

    int totalCitiesCount;
    int totalRoadsCount;
    int totalStatesCount;

    if (fscanf(inputFilePointer, "%d %d", &totalCitiesCount, &totalRoadsCount) != 2) {
        puts("Ошибка: неверный формат файла");
        fclose(inputFilePointer);
        return 1;
    }

    if (totalCitiesCount > DEFAULT_MAX_CITIES || totalCitiesCount <= 0) {
        printf("Ошибка: недопустимое количество городов: %d (макс. %d)\n", totalCitiesCount, DEFAULT_MAX_CITIES);
        fclose(inputFilePointer);
        return 1;
    }

    Road* roadsArray = malloc(totalRoadsCount * sizeof(Road));
    if (!roadsArray) {
        puts("Ошибка: не удалось выделить память");
        fclose(inputFilePointer);
        return 1;
    }

    for (int currentRoadIndex = 0; currentRoadIndex < totalRoadsCount; ++currentRoadIndex) {
        if (fscanf(inputFilePointer, "%d %d %d",
                &roadsArray[currentRoadIndex].cityFrom,
                &roadsArray[currentRoadIndex].cityTo,
                &roadsArray[currentRoadIndex].roadLength)
            != 3) {
            puts("Ошибка: неверный формат дороги");
            free(roadsArray);
            fclose(inputFilePointer);
            return 1;
        }
        roadsArray[currentRoadIndex].cityFrom--;
        roadsArray[currentRoadIndex].cityTo--;
    }

    if (fscanf(inputFilePointer, "%d", &totalStatesCount) != 1) {
        puts("Ошибка: неверное количество государств");
        free(roadsArray);
        fclose(inputFilePointer);
        return 1;
    }

    if (totalStatesCount > totalCitiesCount || totalStatesCount <= 0) {
        printf("Ошибка: недопустимое количество государств: %d\n", totalStatesCount);
        free(roadsArray);
        fclose(inputFilePointer);
        return 1;
    }

    int* capitalsArray = malloc(totalStatesCount * sizeof(int));
    if (!capitalsArray) {
        puts("Ошибка: не удалось выделить память");
        free(roadsArray);
        fclose(inputFilePointer);
        return 1;
    }

    int* capitalCityUsed = calloc(totalCitiesCount, sizeof(int));
    if (!capitalCityUsed) {
        puts("Ошибка: не удалось выделить память");
        free(roadsArray);
        free(capitalsArray);
        fclose(inputFilePointer);
        return 1;
    }

    for (int currentStateIndex = 0; currentStateIndex < totalStatesCount; ++currentStateIndex) {
        if (fscanf(inputFilePointer, "%d", &capitalsArray[currentStateIndex]) != 1) {
            puts("Ошибка: неверный формат столицы");
            free(roadsArray);
            free(capitalsArray);
            free(capitalCityUsed);
            fclose(inputFilePointer);
            return 1;
        }
        capitalsArray[currentStateIndex]--;

        if (capitalsArray[currentStateIndex] < 0 || capitalsArray[currentStateIndex] >= totalCitiesCount) {
            printf("Ошибка: недопустимый индекс столицы: %d\n", capitalsArray[currentStateIndex] + 1);
            free(roadsArray);
            free(capitalsArray);
            free(capitalCityUsed);
            fclose(inputFilePointer);
            return 1;
        }

        if (capitalCityUsed[capitalsArray[currentStateIndex]]) {
            printf("Ошибка: дублирование столицы: город %d\n", capitalsArray[currentStateIndex] + 1);
            free(roadsArray);
            free(capitalsArray);
            free(capitalCityUsed);
            fclose(inputFilePointer);
            return 1;
        }
        capitalCityUsed[capitalsArray[currentStateIndex]] = 1;
    }

    int firstCapitalCity = capitalsArray[0];

    free(capitalCityUsed);

    int* distanceMatrix = malloc(totalCitiesCount * totalCitiesCount * sizeof(int));
    int* cityToStateMap = malloc(totalCitiesCount * sizeof(int));
    StateCities* statesArray = malloc(totalStatesCount * sizeof(StateCities));
    int* visitedCities = calloc(totalCitiesCount, sizeof(int));
    int* bfsQueue = malloc(totalCitiesCount * sizeof(int));

    if (!distanceMatrix || !cityToStateMap || !statesArray || !visitedCities || !bfsQueue) {
        puts("Ошибка: не удалось выделить память");
        free(roadsArray);
        free(capitalsArray);
        free(distanceMatrix);
        free(cityToStateMap);
        free(statesArray);
        free(visitedCities);
        free(bfsQueue);
        fclose(inputFilePointer);
        return 1;
    }

    for (int matrixIndex = 0; matrixIndex < totalCitiesCount * totalCitiesCount; ++matrixIndex) {
        distanceMatrix[matrixIndex] = INFINITY_DISTANCE;
    }
    for (int cityIndex = 0; cityIndex < totalCitiesCount; ++cityIndex) {
        distanceMatrix[cityIndex * totalCitiesCount + cityIndex] = 0;
        cityToStateMap[cityIndex] = INVALID_STATE;
    }

    for (int stateIndex = 0; stateIndex < totalStatesCount; ++stateIndex) {
        statesArray[stateIndex].citiesArray = malloc(totalCitiesCount * sizeof(int));
        statesArray[stateIndex].citiesCount = 0;
        statesArray[stateIndex].citiesCapacity = totalCitiesCount;
        if (!statesArray[stateIndex].citiesArray) {
            puts("Ошибка: не удалось выделить память");
            return 1;
        }
    }

    for (int currentRoadIndex = 0; currentRoadIndex < totalRoadsCount; ++currentRoadIndex) {
        int roadCityFrom = roadsArray[currentRoadIndex].cityFrom;
        int roadCityTo = roadsArray[currentRoadIndex].cityTo;
        int roadLength = roadsArray[currentRoadIndex].roadLength;

        if (roadCityFrom >= 0 && roadCityFrom < totalCitiesCount && roadCityTo >= 0 && roadCityTo < totalCitiesCount) {
            distanceMatrix[roadCityFrom * totalCitiesCount + roadCityTo] = roadLength;
            distanceMatrix[roadCityTo * totalCitiesCount + roadCityFrom] = roadLength;
        }
    }

    free(roadsArray);
    fclose(inputFilePointer);

    for (int currentStateIndex = 0; currentStateIndex < totalStatesCount; ++currentStateIndex) {
        int capitalCityIndex = capitalsArray[currentStateIndex];
        cityToStateMap[capitalCityIndex] = currentStateIndex;
        statesArray[currentStateIndex].citiesArray[statesArray[currentStateIndex].citiesCount++] = capitalCityIndex;
    }

    free(capitalsArray);

    int bfsQueueHead = 0;
    int bfsQueueTail = 0;

    visitedCities[firstCapitalCity] = 1;
    bfsQueue[bfsQueueTail++] = firstCapitalCity;

    while (bfsQueueHead < bfsQueueTail) {
        int currentCityIndex = bfsQueue[bfsQueueHead++];
        for (int neighborCityIndex = 0; neighborCityIndex < totalCitiesCount; ++neighborCityIndex) {
            if (!visitedCities[neighborCityIndex] && distanceMatrix[currentCityIndex * totalCitiesCount + neighborCityIndex] != INFINITY_DISTANCE) {
                visitedCities[neighborCityIndex] = 1;
                bfsQueue[bfsQueueTail++] = neighborCityIndex;
            }
        }
    }

    int unreachableCitiesCount = 0;
    for (int cityIndex = 0; cityIndex < totalCitiesCount; ++cityIndex) {
        if (!visitedCities[cityIndex]) {
            ++unreachableCitiesCount;
        }
    }

    if (unreachableCitiesCount > 0) {
        printf("Предупреждение: обнаружено недостижимых городов: %d\n", unreachableCitiesCount);
    }

    int distributedCitiesCount = totalStatesCount;
    int currentStateIndex = 0;
    int consecutiveStatesWithoutCityCount = 0;

    while (distributedCitiesCount < totalCitiesCount) {
        int bestCandidateCityIndex = NO_CITY_FOUND;
        int bestRoadLength = INFINITY_DISTANCE;

        for (int cityInStateIndex = 0; cityInStateIndex < statesArray[currentStateIndex].citiesCount; ++cityInStateIndex) {
            int currentstateCity = statesArray[currentStateIndex].citiesArray[cityInStateIndex];

            for (int neighborCityIndex = 0; neighborCityIndex < totalCitiesCount; ++neighborCityIndex) {
                if (cityToStateMap[neighborCityIndex] != INVALID_STATE) {
                    continue;
                }

                int currentRoadLength = distanceMatrix[currentstateCity * totalCitiesCount + neighborCityIndex];
                if (currentRoadLength == INFINITY_DISTANCE) {
                    continue;
                }

                if (currentRoadLength < bestRoadLength || (currentRoadLength == bestRoadLength && neighborCityIndex < bestCandidateCityIndex)) {
                    bestRoadLength = currentRoadLength;
                    bestCandidateCityIndex = neighborCityIndex;
                }
            }
        }

        if (bestCandidateCityIndex != NO_CITY_FOUND) {
            cityToStateMap[bestCandidateCityIndex] = currentStateIndex;
            statesArray[currentStateIndex].citiesArray[statesArray[currentStateIndex].citiesCount++] = bestCandidateCityIndex;
            ++distributedCitiesCount;
            consecutiveStatesWithoutCityCount = 0;
        } else {
            ++consecutiveStatesWithoutCityCount;
            if (consecutiveStatesWithoutCityCount >= totalStatesCount) {
                printf("Предупреждение: не удалось распределить городов: %d (несвязный граф)\n",
                    totalCitiesCount - distributedCitiesCount);
                break;
            }
        }

        currentStateIndex = (currentStateIndex + 1) % totalStatesCount;
    }

    for (int currentStateIndex = 0; currentStateIndex < totalStatesCount; ++currentStateIndex) {
        printf("Государство %d:", currentStateIndex + 1);

        for (int cityIndex = 0; cityIndex < totalCitiesCount; ++cityIndex) {
            if (cityToStateMap[cityIndex] == currentStateIndex) {
                printf(" %d", cityIndex + 1);
            }
        }
        puts("");
    }

    for (int stateIndex = 0; stateIndex < totalStatesCount; ++stateIndex) {
        free(statesArray[stateIndex].citiesArray);
    }
    free(statesArray);
    free(distanceMatrix);
    free(cityToStateMap);
    free(visitedCities);
    free(bfsQueue);

    return 0;
}
