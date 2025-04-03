bfs=$(cat $1)

average_bf=0
count=0

for bf in $bfs; do
    (( average_bf += $bf ))
    (( count += 1 ))
done

echo "Count: $count"
echo "Result: $(( $average_bf / $count ))"
