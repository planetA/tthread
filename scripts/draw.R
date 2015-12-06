library(DBI)
library(RSQLite)
library(data.table)

con = dbConnect(RSQLite::SQLite(), dbname="./out/sim.db")
alltables = dbListTables(con)
print(alltables)


dt <- data.table(dbGetQuery(con, 'select * from runtime'))

library(ggplot2)

pdf('/tmp/out.pdf')

threads = unique(dt$threads)
for (i in threads) {
    c <- ggplot(dt[threads==i],
                aes(x=factor(app), y=time, fill=sched))
    print(c + geom_bar(stat="identity", position="dodge") +
          facet_wrap( ~ threads, ncol=1))
}

dev.off()
