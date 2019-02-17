FROM petronetto/docker-python-deep-learning
ENV PASSWORD=Palle9912
RUN cd /home/notebooks
RUN apt-get update 
RUN apt-get install -y git 
RUN git clone https://8f6b6a81214b3855aab1a65e7a093fe44104bc0e@github.com/antoniomuso/PeterPen.git
RUN pip install pydot && pip install dtw
WORKDIR /home/notebooks/PeterPen

CMD git pull ; jupyter notebook --port=8888 --no-browser --allow-root --ip=0.0.0.0 --NotebookApp.token=${PASSWORD}
