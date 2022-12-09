echo "22" > /sys/class/gpio/unexport
echo "22" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio22/direction

while true
do
	curr_date=$(date +"%T.%6N")
	curr_val=$(cat /sys/class/gpio/gpio22/value)
#	curr_val=$(cat gpio_value)
	echo $curr_date $curr_val >> button_data &
	sleep 1
done
