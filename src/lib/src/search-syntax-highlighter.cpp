#include "search-syntax-highlighter.h"
#include <QColor>
#include <QRegularExpression>
#include "models/profile.h"


SearchSyntaxHighlighter::SearchSyntaxHighlighter(bool full, QTextDocument *parent, Profile *profile)
	: QSyntaxHighlighter(parent), m_profile(profile)
{
	HighlightingRule rule;

	// Or format "~tag"
	rule.pattern = QRegularExpression("(?: |^)~([^ ]+)");
	rule.format.setForeground(Qt::green);
	highlightingRules.append(rule);

	// Exclusion format "-tag"
	rule.pattern = QRegularExpression("(?: |^)-([^ ]+)");
	rule.format.setForeground(Qt::red);
	highlightingRules.append(rule);

	if (!full) {
		// Meta other format "unknown_meta:value"
		rule.pattern = QRegularExpression("(?: |^)([^:]+):([^: ][^ ]*)?");
		rule.format.setForeground(QColor("#ff0000")); // red
		highlightingRules.append(rule);
	} else {
		// MD5 format "qdrg15sdfgs1d2f1gs3dfg"
		rule.pattern = QRegularExpression("(?: |^)([0-9A-F]{32})", QRegularExpression::CaseInsensitiveOption);
		rule.format.setForeground(QColor("#800080")); // purple
		highlightingRules.append(rule);

		// URL format "http://..."
		rule.pattern = QRegularExpression("(?: |^)(https?://[^\\s/$.?#].[^\\s]*)");
		rule.format.setForeground(Qt::blue);
		highlightingRules.append(rule);
	}

	// Meta format "meta:value"
	rule.pattern = QRegularExpression("(?: |^)(user|fav|md5|pool|rating|source|status|approver|unlocked|sub|id|width|height|score|mpixels|filesize|filetype|date|gentags|arttags|chartags|copytags|status|status|approver|order|parent|sort|grabber):([^: ][^ ]*)?", QRegularExpression::CaseInsensitiveOption);
	rule.format.setForeground(QColor("#a52a2a")); // brown
	highlightingRules.append(rule);

	if (m_profile != nullptr) {
		// Favorites format "favorited_tag"
		rule.format.setForeground(QColor("#ffc0cb")); // pink
		highlightingRules.append(rule);
		m_favoritesRule = &highlightingRules.last();

		// Favorites format "kfl_tag"
		rule.format.setForeground(QColor("#000000")); // black
		highlightingRules.append(rule);
		m_kflRule = &highlightingRules.last();

		updateFavorites();
		updateKeptForLater();
		connect(m_profile, &Profile::favoritesChanged, this, &SearchSyntaxHighlighter::updateFavorites);
		connect(m_profile, &Profile::keptForLaterChanged, this, &SearchSyntaxHighlighter::updateKeptForLater);
	}
}

void SearchSyntaxHighlighter::updateFavorites()
{
	QString favorites;
	for (const auto &favorite : m_profile->getFavorites()) {
		if (!favorites.isEmpty()) {
			favorites += '|';
		}
		favorites += favorite.getName();
	}
	m_favoritesRule->pattern.setPattern("(?: |^)(" + favorites + ")");
}

void SearchSyntaxHighlighter::updateKeptForLater()
{
	m_kflRule->pattern.setPattern("(?: |^)(" + m_profile->getKeptForLater().join('|') + ")");
}

void SearchSyntaxHighlighter::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : highlightingRules) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
}
