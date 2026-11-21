# hawkes-market-making
Evaluates whether modeling self-exciting, order flow imbalance-driven arrivals improves a market maker's profit and loss net of adverse selection.

## Data input format

The project expects LOBSTER-style order book event rows. Each input row must contain the following columns in this order:

1. Time: seconds after midnight, with decimal precision (for example 12.345678)
2. Type: event type code
   - 1 = Submission of a new limit order
   - 2 = Cancellation (partial deletion of a limit order)
   - 3 = Deletion (total deletion of a limit order)
   - 4 = Execution of a visible limit order
   - 5 = Execution of a hidden limit order
   - 7 = Trading halt indicator
3. Order ID: unique order reference number
4. Size: number of shares
5. Price: integer price in ticks, where the value is scaled by 10000; for example, $91.14 is stored as 911400
6. Direction: order side
   - 1 = Buy
   - -1 = Sell

Example row:

```csv
12.345678,1,1042,100,911400,1
```

This means: a buy limit order at $91.14, size 100, order ID 1042, at time 12.345678 seconds after midnight.

The parser currently expects CSV rows with six fields and uses the above schema to populate the internal event structure.
