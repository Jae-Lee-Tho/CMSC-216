#!/bin/bash
#
# Shows examples of running scale_main on various sensor values


# echo '> make thermo_main' 
make scale_main > /dev/null
echo

cmds=()
cmds+=("./scale_main")
cmds+=("./scale_main 0 0 oz tare_no")
cmds+=("./scale_main 5 0 oz tare_no")
cmds+=("./scale_main 839 0 oz tare_no")
cmds+=("./scale_main 839 125 oz tare_no")
cmds+=("./scale_main 0 125 oz tare_no")
cmds+=("./scale_main 839 125 lb tare_no")
cmds+=("./scale_main 839 125 lb tare_yes")
cmds+=("./scale_main -15 0 oz tare_no")
cmds+=("./scale_main 0 -48 lb tare_no")
cmds+=("./scale_main 0 -48 oz tare_yes")


i=1
for cmd in "${cmds[@]}"; do
    printf "########## EXAMPLE %2d ##########\n" "$i"
    printf ">> %s\n" "$cmd"
    $cmd
    printf "\n\n"
    ((i++))
done
