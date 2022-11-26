int button_state;
pipe(button_state)
if [$button_state = 0]
then
	dt=$(sudo rangefinder_hcsr04/rangefinder_hcsr04 -q 10000000)
	char note;
	if (( $val > 0.005))
	then
		note = 'A'
		play_note/play_note -q
		echo $note
	fi
fi
