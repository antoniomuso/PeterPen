FROM ufoym/deepo:all-jupyter-py36-cu90
ENV PASSWORD=Palle9912
#Install git
RUN apt-get update \
    && apt-get install -y git
 
#Change directory and clone Qxf2 Public POM repo
RUN mkdir /usr/PeterPen \
    && cd /usr/PeterPen \
    && git clone https://8f6b6a81214b3855aab1a65e7a093fe44104bc0e@github.com/antoniomuso/PeterPen.git \
    && pip install pydot \
    && pip install dtw \
    && pip install mlpy \
    && mkdir /usr/Graph

WORKDIR /usr/PeterPen/PeterPen
CMD git pull; tensorboard --logdir /usr/Graph & jupyter notebook --port=8888 --no-browser --allow-root  --ip=0.0.0.0 --NotebookApp.token=${PASSWORD}
