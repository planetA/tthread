library(DBI)
library(RSQLite)
library(data.table)

con = dbConnect(RSQLite::SQLite(), dbname="./out/sim.db")
alltables = dbListTables(con)
print(alltables)


dt <- data.table(dbGetQuery(con, 'select * from runtime'))

library(ggplot2)

pdf('/tmp/out.pdf')

cpulist = unique(dt$cpulist)
for (i in cpulist) {
    c <- ggplot(dt[cpulist==i],
                aes(x=factor(app), y=time, fill=sched)) +
        theme(axis.text.x = element_text(angle = 90, hjust = 1))
    print(c + geom_bar(stat="identity", position="dodge") +
          facet_wrap( ~ cpulist, ncol=1))
}


apps = unique(dt$app)
for (i in apps) {
    c <- ggplot(dt[app==i],
                aes(x=factor(cpulist), y=time, fill=sched))+
        theme(axis.text.x = element_text(angle = 90, hjust = 1))
    print(c + geom_bar(stat="identity", position="dodge") +
          facet_wrap( ~ app, ncol=1))
}

dev.off()
