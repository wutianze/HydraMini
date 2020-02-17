COLLECT = collect
RUN = run
FPT = fpt

CUR_DIR =   $(shell pwd)
SRC     =   $(CUR_DIR)/src
BUILD   =   $(CUR_DIR)/build
MODEL   =   $(CUR_DIR)/model/dpu_testModel.elf
YOLO    =   $(CUR_DIR)/model/yolo_model.elf
ARCH    =     $(shell uname -m | sed -e s/arm.*/armv71/ \
	-e s/aarch64.*/aarch64/ )

#vpath %.o $(BUILD)
#vpath %.cc $(SRC)

CXX       :=   g++
CC        :=   gcc 
COLLECTOBJ :=   collect.o control.o
RUNOBJ    :=   run.o control.o
FPTOBJ    := control.o cv_lane.o runYolo.o run_fpt.o
# linking libraries of OpenCV
COLLECTLDFLAGS   =   $(shell pkg-config --libs opencv) -lpthread
RUNLDFLAGS  =  $(COLLECTLDFLAGS) -lhineon -ln2cube -ldputils

CFLAGS  :=  -O2 -Wall -Wpointer-arith -std=c++11 -ffast-math

ifeq ($(ARCH),armv71)
	CFLAGS +=  -mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon
endif
ifeq ($(ARCH),aarch64)
	CFLAGS += -mcpu=cortex-a53
endif

.PHONY: clean

$(BUILD)/%.o : $(SRC)/%.cc
	$(CXX) -c $(CFLAGS) $< -o $@


$(COLLECT) : $(addprefix $(BUILD)/, $(COLLECTOBJ))
	$(CXX) $(CFLAGS) $^ -o $(BUILD)/$@ $(COLLECTLDFLAGS)

$(RUN) : $(addprefix $(BUILD)/, $(RUNOBJ))
	$(CXX) $(CFLAGS) $^ $(MODEL) -o $(BUILD)/$@ $(RUNLDFLAGS)

$(FPT) : $(addprefix $(BUILD)/, $(FPTOBJ))
	$(CXX) $(CFLAGS) $^ $(YOLO) -o $(BUILD)/$@ $(RUNLDFLAGS)

clean:
	$(RM) -rf $(BUILD)/*
