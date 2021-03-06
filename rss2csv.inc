<?php
/* rss2csv.inc - utilities for RSS->CSV conversion script

   Created by Brian Cantoni <brian AT cantoni.org>
   scooterlabs.com/hacks/rss2csv.php
*/

// max cell size for benefit of MS Excel
// ref: https://support.office.com/en-us/article/Excel-specifications-and-limits-1672b34d-7043-467e-8e27-269d656771c3
define ('MAX_CELL_SIZE', 32000);

// given RSS feed URL, fetch it using YQL, then output as CSV
function fetchRSSandOutputCSV ($url) {
    $yqlBase = "http://query.yahooapis.com/v1/public/yql";
    $query = "select title,link,description,pubDate,guid from rss where url='$url'";
    $queryUrl = $yqlBase . "?q=" . rawurlencode($query) . "&format=json";

    $curl = curl_init($queryUrl);
    curl_setopt($curl, CURLOPT_HEADER, false);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    $json = curl_exec($curl);

    curl_close($curl);
    $result = json_decode($json);

    $items = $result->{'query'}->{'results'}->{'item'};

    header('Content-Type: text/csv; charset=utf-8');
    header('Content-Disposition: attachment; filename="export.csv"');
    header('Pragma: no-cache');
    header('Expires: 0');

    $count = $result->{'query'}->{'count'};
    if ($count == 0) {
        print "No data returned for RSS feed\n";
        return;
    }

    // save values from first items (to use as column headers)
    $headers = array();
    foreach ($items[0] as $k => $v) {
        $headers[] = $k;
    }

    // open stdout and write headers, then data, as CSV
    $fp = fopen('php://output', 'w');
    if ($fp && $result) {
        fputcsv($fp, $headers);
        foreach ($items as $item) {
            $values = Array();
            foreach ($item as $k => $v) {
                if (is_scalar($v)) { // only write out scalars (skip objects, arrays, etc.)
                    $values[] = substr($v, 0, MAX_CELL_SIZE);
                } else {
                    $values[] = '';
                }
            }
            fputcsv($fp, $values);
        }
    }
    return;
}
