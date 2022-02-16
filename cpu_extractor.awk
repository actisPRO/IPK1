{
    if ($1 == "cpu") {
        idleAll = $4 + $5
        nonIdleAll = $1 + $2 + $3 + $6 + $7 + $8
        total = idleAll + nonIdleAll
        printf "%s %s %s\n", idleAll, nonIdleAll, total
    }
}