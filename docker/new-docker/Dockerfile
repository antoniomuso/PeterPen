FROM ufoym/deepo:all-jupyter-py36-cu90
ENV PASSWORD=Balle9912
ENV DIR_NAME=PeterPen
#Install git
RUN apt-get update \
    && apt-get install -y git \
    && mkdir /${DIR_NAME}
WORKDIR /${DIR_NAME}
RUN pip install pip --upgrade \
    && pip install notebook --upgrade \
    && pip install pydot \
    && pip install dtw \
    && mkdir /usr/Graph \
    && pip install Cython \
    && pip install cydtw \
    && pip install fastdtw 
CMD tensorboard --logdir /usr/Graph & jupyter notebook --port=8888 --no-browser --allow-root --ip=0.0.0.0 --NotebookApp.token=${PASSWORD}
