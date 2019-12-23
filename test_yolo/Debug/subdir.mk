################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../detectionpictures.cpp 

OBJS += \
./detectionpictures.o 

CPP_DEPS += \
./detectionpictures.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/data/006zzy/files/project5-master/1018CPUGPUBSV6/detection-master-v6/include -include/data/006zzy/files/project5-master/1018CPUGPUBSV6/detection-master-v6/include/detection.h -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


