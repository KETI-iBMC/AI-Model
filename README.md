# AI-Model


start setup.sh 


source ~/zephyrproject/.venv/bin/activate

pip install west

west init ~/zephyrproject

cd ~/zephyrproject

west update

west zephyr-export

pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
