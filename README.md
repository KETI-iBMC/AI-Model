# AI-Model


setup.sh 실행


source ~/zephyrproject/.venv/bin/activate

pip install west

west init ~/zephyrproject

cd ~/zephyrproject

west update

west zephyr-export

pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
