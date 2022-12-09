#!/bin/bash
notes_num=12
max_len=1000
min_len=100
let "len_step = ($max_len - $min_len) / $notes_num"
echo len_step = $len_step
while true
do
	button_data_read=$(cat button_data)
	button_state="${button_data_read: -1}"
	echo button_state = $button_state
	if [ $button_state = 1 ]
	then
		dt_temp=$(sudo ./rangefinder_hcsr04 -q 10000000)
		#dt_temp=$(cat rangefinder_val)
		echo dt_temp = $dt_temp
		dt=$(echo $dt_temp*1000000 | bc)
		dt=${dt%.*}

		if [ $dt -ge $max_len ]
		then
			echo -e "A#" | ./play_note -q
		elif [ $dt -ge $(( $max_len-$len_step )) ]
		then
			  echo -e "A" | ./play_note -q
		elif [ $dt -ge $(( $max_len-2*$len_step )) ]
			then
				  echo -e "B" | ./play_note -q
			elif [ $dt -ge $(( $max_len-3*$len_step )) ]
			then
				  echo -e "C#" | ./play_note -q
			elif [ $dt -ge $(( $max_len-4*$len_step )) ]
			then
				  echo -e "C" | ./play_note -q
		elif [ $dt -ge $(( $max_len-5*$len_step )) ]
			then
				  echo -e "D#" | ./play_note -q
		elif [ $dt -ge $(( $max_len-6*$len_step )) ]
			then
				  echo -e "D" | ./play_note -q
		elif [ $dt -ge $(( $max_len-7*$len_step )) ]
			then
				  echo -e "E" | ./play_note -q
		elif [ $dt -ge $(( $max_len-8*$len_step )) ]
			then
				  echo -e "F#" | ./play_note -q
		elif [ $dt -ge $(($max_len-9*$len_step )) ]
			then
				  echo -e "F" | ./play_note -q
		elif [ $dt -ge $(($max_len-10*$len_step )) ]
			then
				  echo -e "G#" | ./play_note -q
		elif [ $dt -le $(($max_len-10*$len_step )) ]
			then
				  echo -e "G" | ./play_note -q
		else
			echo dt = $dt
			fi
	else
		min_len_temp=$(sudo ./rangefinder_hcsr04 -q 10000000)
		min_len=$(echo $min_len_temp*1000000 | bc)
		min_len=${min_len%.*}
		#min_len=$(cat rangefinder_val)
		echo "set min len = " $min_len
		let "len_step = ($max_len - $min_len) / $notes_num"
		echo len_step = $len_ste
	fi
	
done
