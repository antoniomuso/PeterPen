FROM petronetto/docker-python-deep-learning
RUN cd /home/notebooks
RUN apt-get update 
RUN apt-get install -y git 
RUN git clone https://8f6b6a81214b3855aab1a65e7a093fe44104bc0e@github.com/antoniomuso/PeterPen.git
RUN pip install pydot
WORKDIR /home/notebooks/PeterPen

CMD [ "jupyter", "notebook", "--port=8888", "--no-browser", \
    "--allow-root", "--ip=0.0.0.0", "--NotebookApp.token=Palle9912" ]
